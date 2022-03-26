// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

template <typename pT, int* pS> class Endpoint : public std::enable_shared_from_this<Endpoint<pT, pS>> {
    public:
        // incoming packet queue
        Queue<MetaPacket<pT, pS>> packetsIn;
        // shared across whole endpoint instance
        io_context asioContext;
        std::thread contextThread;
        ip::tcp::acceptor asioAcceptor;
        // will be the public key of node instance
        char publicKey[5];
        uint16_t port;
        // routing table (public key -> ip, port, connection pointer)
        RoutingTable<pT, pS> connections;
        // parent node
        ConnectionData<pT, pS>* parentNode;

        Endpoint(
            char* _publicKey, 
            uint16_t _port
        ) : asioAcceptor(
            asioContext, 
            ip::tcp::endpoint(ip::tcp::v4(), _port)
        ) {
            std::strcpy(publicKey, _publicKey);
            port = _port;
        }

        virtual ~Endpoint() {
            stop();
        }

        // starts the operation of the endpoint
        bool start() {
            try {
                // starting to listen for remote connections
                waitForConnection();
                // launching asio context on its own thread 
                contextThread = std::thread(
                    [this] () { 
                        asioContext.run();
                    }
                );
            } catch (std::exception& e) {
                print::error(std::string("start() - error: ") + std::string(e.what()));
                return false;
            }
            print::info("start(): endpoint started successfully");
            return true;
        }

        void stop() {
            asioContext.stop();
            if (contextThread.joinable()) contextThread.join();
            print::info("stop(): endpoint stopped");
        }

        // async - starts a connection to a remote node and stores it in the connections container
        void connect(
            const std::string& host, 
            uint16_t port, 
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> callback = [](std::shared_ptr<Connection<pT, pS>>){},
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> reject = [](std::shared_ptr<Connection<pT, pS>>){}
        ) {
            try {
                // dns lookup for the hostname
                ip::tcp::resolver resolver(asioContext);
                ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
                // creating connection instance
                std::shared_ptr<Connection<pT, pS>> newConn =
                    std::make_shared<Connection<pT, pS>> (
                        asioContext, 
                        ip::tcp::socket(asioContext), 
                        packetsIn
                    );
                // connecting to the remote node and validating connection
                newConn->remoteConnect(
                    endpoints, 
                    publicKey, 
                    [this, newConn, callback] (
                        std::shared_ptr<Connection<pT, pS>> conn
                    ) {
                        this->connections.set(
                            conn->publicKey,
                            conn->connSocket.remote_endpoint().address().to_string(),
                            conn->connSocket.remote_endpoint().port(),
                            conn->shared_from_this()
                        );
                        callback(conn);
                    }, 
                    [this, newConn, reject] (
                        std::shared_ptr<Connection<pT, pS>> conn
                    ) {
                        this->disconnect(conn->publicKey);
                        //reject(conn);
                    }
                );
            } catch (std::exception& e) {
                print::error(std::string("connect() - error: ") + std::string(e.what()));
            }
        }

        // async - wait for connection
        void waitForConnection() {
            asioAcceptor.async_accept(
                [this] (
                    std::error_code ec, 
                    ip::tcp::socket socket
                ) {
                    if (!ec) {
                        print::debug(std::string("waitForConnection(): new connection from ") 
                            + socket.remote_endpoint().address().to_string());
                        // create new connection to handle client
                        std::shared_ptr<Connection<pT, pS>> newConn =
                            std::make_shared<Connection<pT, pS>> (
                                asioContext, 
                                std::move(socket), 
                                packetsIn
                            );
                        // TODO: revise onNodeConnect()
                        if (onNodeConnect(newConn)) {
                            // considering remote node as connected and validating the connection
                            newConn->localConnect(
                                [this, newConn] (
                                    std::shared_ptr<Connection<pT, pS>> conn
                                ) {
                                    this->connections.set(
                                        conn->publicKey,
                                        conn->connSocket.remote_endpoint().address().to_string(),
                                        conn->connSocket.remote_endpoint().port(),
                                        conn->shared_from_this()
                                    );
                                },
                                [this, newConn] (
                                    std::shared_ptr<Connection<pT, pS>> conn
                                ) {
                                    this->disconnect(conn->publicKey);
                                    //reject(conn);
                                }
                            );
                            print::debug("waitForConnection(): connection successfully catched");
                        } else {
                            print::debug("waitForConnection(): connection rejected");
                        }
                    } else {
                        print::error(std::string("waitForConnection() - error: ") + ec.message());
                    }
                    // recursively starting to wait for other connections
                    waitForConnection();
                }
            );
        }

        // async - makes sure there is a connection with a specified node
        void assureConnection(
            char* _publicKey, 
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> callback = [](std::shared_ptr<Connection<pT, pS>>){}
        ) {
            ConnectionData<pT, pS>* res = connections.get(_publicKey);
            if (!res) {
                print::trace("assureConnection(): could not find node in local routing table");
                queryConnectionData(
                    _publicKey, 
                    [this, callback] (ConnectionData<pT, pS> _node) {
                        this->connect(
                            _node.ipAddress,
                            _node.port,
                            callback
                        );
                    }
                );
                return;
            }
            if (!(res->connection) || !(res->connection->isOpen())) {
                print::trace("assureConnection(): node found in routing table");
                connect(
                    res->ipAddress,
                    res->port,
                    callback
                );
            } else {
                print::trace("assureConnection(): node has active connection");
                callback(res->connection);
            }
        }

        void disconnect(::publicKey _publicKey) {
            print::debug(std::string("disconnect() - disconnecting from ") + std::string(_publicKey));
            ConnectionData<pT, pS>* node = connections.get(_publicKey);
            if (!node) {
                return;
            }
            onNodeDisconnect(node->connection);
            node->connection.reset();
            connections.set(
                _publicKey,
                node->ipAddress,
                node->port
            );
        }

        // async - send a packet to a specified node
        void sendNode(std::shared_ptr<Connection<pT, pS>> remoteNode, Packet<pT, pS>& _packet) {
            if (remoteNode && remoteNode->isOpen()) {
                remoteNode->send(_packet);
            }
        }

        // async - send a packet to a specified nodes
        void sendNode(
            char* _publicKey, 
            Packet<pT, pS>& _packet
        ) {
            assureConnection(
                _publicKey,
                [this, &_packet] (
                    std::shared_ptr<Connection<pT, pS>> node
                ) {
                    if (!node) {
                        print::error("sendNode(): got nullptr for connection");
                        return;
                    }
                    node->send(_packet);
                }
            );
        }

        // 'maxPackets' defines how many packets to process at once
        // if 'wait' is set to true, the thread sleeps until a packet is received
        void update(uint32_t maxPackets = -1, bool wait = false) {
            if (wait) packetsIn.wait();
            uint32_t packetCount = 0;
            while (packetCount < maxPackets && !packetsIn.empty()) {
                auto _packet = packetsIn.pop_front();
                onMessage(_packet);
                packetCount++;
            }
        }

        // event handler - incoming connection (returning false means dropping connection)
        virtual bool onNodeConnect(std::shared_ptr<Connection<pT, pS>> remoteNode) {
            return true;
        }

        // event handler - disconnection of remote node
        virtual void onNodeDisconnect(std::shared_ptr<Connection<pT, pS>> remoteNode) { }

        // event handler - incoming packet
        virtual void onMessage(MetaPacket<pT, pS>& packet) {
            print::info("onMessage(): incoming packet");
            // handler for each packet type
            switch (packet.content.header.packetType) {
                case pT::textMessage: {
                    char s[256];
                    packet.content >> s;
                    std::cout << s << std::endl;
                    break;
                }
                // default case always means programming error
                default: {
                    std::cout << "onMessage() - error: this message should not have got to this handler" << std::endl;
                    break;
                }
            }
        }

        virtual void queryConnectionData(
            char* _publicKey, 
            std::function<void(ConnectionData<pT, pS>)> callback = [](ConnectionData<pT, pS>){}
        ) { }
};