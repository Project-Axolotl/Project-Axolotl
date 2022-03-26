// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

template <typename pT, int* pS> class Connection : public std::enable_shared_from_this<Connection<pT, pS>> {
    public:
        // single instance is shared across the whole node
        io_context& connIOContext;
        ip::tcp::socket connSocket;
        // incoming queue does not need to be separated by sender
        // MetaPacket already has the metadata of the sender
        Queue<MetaPacket<pT, pS>>& connPacketsIn;
        // every connection has separated outgoing packet queue
        Queue<Packet<pT, pS>> connPacketsOut;
        // gets filled with the packet under arrival, which is then copied into its own packet instance
        Packet<pT, pS> connPacketBuffer;
        // will be the public key of the remote node for identification
        char publicKey[5];

        Connection (
            io_context& _IOContext,
            ip::tcp::socket _socket,
            Queue<MetaPacket<pT, pS>>& _packetsIn
        ) :
             connIOContext(_IOContext),
            connSocket(std::move(_socket)),
            connPacketsIn(_packetsIn)  {}

        virtual ~Connection () {}

        // async - connects the socket with a remote node
        // calls validation too which also retrieves the public key of the remote node
        void remoteConnect (
            ip::tcp::resolver::results_type& endpoints, 
            char* localPublicKey, 
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> callback = [](std::shared_ptr<Connection<pT, pS>>){},
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> reject = [](std::shared_ptr<Connection<pT, pS>>){}
        ) {
            print::debug("remoteConnect(): connecting to remote node");
            async_connect(
                connSocket, 
                endpoints, 
                [this, callback, reject] (
                    std::error_code ec, 
                    ip::tcp::endpoint endpoint
                ) {
                    if (!ec) {
                        print::debug("remoteConnect(): connected to remote node");
                        validateNode(callback, reject);
                    } else {
                        print::error("remoteConnect() - error: " + ec.message());
                        reject(this->shared_from_this());
                    }
                }
            );
        }

        // async - calls validation for incoming connection
        void localConnect (
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> callback = [](std::shared_ptr<Connection<pT, pS>>){},
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> reject = [](std::shared_ptr<Connection<pT, pS>>){}
        ) {
            if (connSocket.is_open()) {
                validateNode(callback, reject);
            }
        }

        // async - sends a validation packet and waits for one too from the remote node
        // the validation packet also contains the public key of the remote node
        void validateNode (
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> callback = [](std::shared_ptr<Connection<pT, pS>>){},
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> reject = [](std::shared_ptr<Connection<pT, pS>>){}
        ) {
            // reading packet header
            async_read(
                connSocket, 
                buffer(&connPacketBuffer.header, sizeof(PacketHeader<pT, pS>)),
                [this, callback, reject] (
                    std::error_code _ec, 
                    std::size_t length
                ) mutable {
                    if (!_ec && connPacketBuffer.header.packetType == pT::nodeValidation) {
                        print::trace("validateNode(): validation header type is correct");
                        connPacketBuffer.body.resize(connPacketBuffer.header.bodySize());
                        // reading packet body
                        async_read(
                            connSocket, 
                            buffer(connPacketBuffer.body.data(), connPacketBuffer.header.bodySize()),
                            [this, callback, reject] (
                                std::error_code __ec, 
                                size_t _length
                            ) mutable {
                                if (!__ec) {
                                    char protocolVer[14];
                                    connPacketBuffer >> protocolVer;
                                    print::trace(std::string("validateNode(): matching protocol version ") + std::string(protocolVer));
                                    if (!std::strcmp(protocolVer, NODE_VERSION)) {
                                        connPacketBuffer >> publicKey;
                                        print::trace(std::string("validateNode(): has publicKey \"") + std::string(publicKey) + std::string("\""));
                                        /* connRoutingTable.insert(std::pair< char*, std::pair<std::string, uint16_t> >
                                            (publicKey, std::pair<std::string, uint16_t> (
                                                connSocket.remote_endpoint().address().to_string(),
                                                connSocket.remote_endpoint().port()
                                            ))); */
                                        print::trace(connSocket.remote_endpoint().address().to_string() 
                                            + std::to_string(connSocket.remote_endpoint().port()));
                                        /* if packet type and protocol version is matching, 
                                            the connection is considered fully established, 
                                            and is being started to be listened on */
                                        read(reject);
                                        callback(this->shared_from_this());
                                        std::strcpy(REMOTE_PUBLIC_KEY, publicKey);
                                    } else {
                                        print::error(std::string("validateNode() - error: unsupported protocol version \"") 
                                            + std::string(protocolVer) + std::string("\""));
                                        reject(this->shared_from_this());
                                    }
                                } else {
                                    print::error(std::string("validateNode() - error: ") + __ec.message());
                                    reject(this->shared_from_this());
                                }
                            });
                        } else {
                            if (_ec) print::error(std::string("validateNode() - error: ") + _ec.message());
                            print::error("validateNode() - error: remote node validation failed");
                            reject(this->shared_from_this());
                        }  
                    });
            // constructing and sending validation packet
            Packet<pT, pS> p;
            p.header.packetType = pT::nodeValidation;
            p << PUBLIC_KEY;
            p << NODE_VERSION;
            send(p, [](std::shared_ptr<Connection<pT, pS>>){});
        }

        bool isOpen() {
            return connSocket.is_open();
        }

        void disconnect() {
            if (isOpen()) { 
                post(
                    connIOContext, 
                    [this] () {
                        print::info("disconnect(): closing connection");
                        connSocket.close();
                    }
                );
            }
        }

        // async - send packet to remote node
        // pushes packet to outgoing queue and if processing is stopped starts it
        void send(
            const Packet<pT, pS>& _packet,
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> reject = [](std::shared_ptr<Connection<pT, pS>>){}
        ) {
            print::trace("send(): sending packets");
            post(
                connIOContext, 
                [this, _packet, reject] () {
                    bool writingPacket = !connPacketsOut.empty();
                    connPacketsOut.push_back(_packet);
                    if (!writingPacket) writeFromQueue(reject);
                }
            );
        }

        // async - processes outgoing packet queue
        // serially writes all packets to socket
        void writeFromQueue(
            std::function<void(std::shared_ptr<Connection<pT, pS>>)> reject = [](std::shared_ptr<Connection<pT, pS>>){}
        ) {
            // writes header of the first packet in the queue
            async_write(connSocket, buffer(&connPacketsOut.front().header, sizeof(PacketHeader<pT, pS>)),
                [this, reject](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        print::trace("writeFromQueue(): header wrote successfully");
                        // write body only if packet has one
                        if (connPacketsOut.front().header.bodySize() > 0) {
                            print::trace(std::string("writeFromQueue(): packet has body (")
                                + std::to_string(connPacketsOut.front().header.bodySize()) + std::string(" bytes)"));
                            // writes body of the first packet in the queue
                            async_write(
                                connSocket, 
                                buffer(connPacketsOut.front().body.data(), 
                                connPacketsOut.front().header.bodySize()),
                                [this, reject] (std::error_code _ec, std::size_t _length) {
                                    if (_ec) {
                                        print::error(std::string("writeFromQueue() - error: ") + _ec.message());
                                        reject(this->shared_from_this());
                                    }
                                    else print::trace("writeFromQueue(): body wrote successfully");
                                    // removes the sent packet from the queue
                                    connPacketsOut.pop_front();
                                    // recursively calls this function
                                    if (!connPacketsOut.empty()) writeFromQueue();
                                }
                            );
                        } else {
                            // removes the sent packet from the queue
                            connPacketsOut.pop_front();
                            // recursively calls this function
                            if (!connPacketsOut.empty()) writeFromQueue();
                        }
                    } else {
                        print::error(std::string("writeFromQueue() - error: ") + ec.message());
                        reject(this->shared_from_this());
                    }
                });
        }

        // async - reads incoming messages
        // waits for a valid packet to be wrote on the socket and then pushes it to the incoming queue
        // basically completes the writeFromQueue() method on the remote side
        void read(std::function<void(std::shared_ptr<Connection<pT, pS>>)> reject = [](std::shared_ptr<Connection<pT, pS>>){}) {
            async_read(
                connSocket, 
                buffer(
                    &connPacketBuffer.header, 
                    sizeof(PacketHeader<pT, pS>)
                ),
                [this, reject] (std::error_code ec, std::size_t length) {
                    if (!ec) {
                        print::trace("read(): header read successfully");
                        if (connPacketBuffer.header.bodySize() > 0) {
                            print::trace(std::string("read(): packet has body (") + 
                                std::to_string(connPacketBuffer.header.bodySize()) + std::string(" bytes)"));
                            connPacketBuffer.body.resize(connPacketBuffer.header.bodySize());
                            async_read(
                                connSocket, 
                                buffer(
                                    connPacketBuffer.body.data(), 
                                    connPacketBuffer.header.bodySize()
                                ),
                                [this, reject] (std::error_code _ec, size_t _length) {
                                    if (!_ec) {
                                        print::trace("read(): body read successfully");
                                        // pushes the complete packet to the incoming queue
                                        addToIncomingPacketQueue();
                                        read();
                                    } else {
                                        print::error(std::string("read() - error: ") + _ec.message());
                                        reject(this->shared_from_this());
                                    }
                                });
                        } else {
                            // pushes the complete packet to the incoming queue
                            addToIncomingPacketQueue();
                            read();
                        }
                    } else {
                        print::error(std::string("read() - error: ") + ec.message());
                        reject(this->shared_from_this());
                    }
                }
            );
        }

        // forms an independent metaPacket from the packet buffer and pushes it to the incoming queue
        void addToIncomingPacketQueue() {
            MetaPacket<pT, pS> p;
            p.content = connPacketBuffer;
            p.packetConn = this->shared_from_this();
            std::strcpy(p.senderPublicKey, publicKey);
            connPacketsIn.push_back(p);
            print::trace("addToIncomingMessageQueue(): successfully processed incoming packet");
        }
};