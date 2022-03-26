// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#define NODE_VERSION "axolotl_alpha"

#include "cli/common.hpp"
#include "networking/common.hpp"
#include "cryptography/common.hpp"
using namespace cli;

// list of all packet types
enum class pT {
    nodeValidation,
    textMessage
};

// size in bytes for all packet types
int pS[] = {
    19,
    256
};

class Node : public Endpoint<pT, pS> {
    public:
        using Endpoint::Endpoint;

        // event handler - incoming connection (return value false means dropping connection)
        virtual bool onNodeConnect(std::shared_ptr<Connection<pT, pS>> _remoteNode) {
            return true;
        }

        // event handler - disconnection of remote node
        virtual void onNodeDisconnect(std::shared_ptr<Connection<pT, pS>> _remoteNode) {

        }

        // event handler - incoming packet (handle incoming packets here)
        virtual void onMessage(MetaPacket<pT, pS>& _packet) {
            print::info("onMessage(): incoming packet");
            // handler for each packet type
            switch (_packet.content.header.packetType) {
                case pT::textMessage: {
                    char s[256];
                    _packet.content >> s;
                    print::notice(
                        std::string(_packet.senderPublicKey) + 
                        std::string(": ") + 
                        std::string(s)
                    );
                    break;
                }
                // default case always means programming error
                default: {
                    print::error("onMessage() - error: this message should not have got to this handler");
                    std::cout << _packet;
                    break;
                }
            }
        }

        virtual void queryConnectionData(
            char* _publicKey, 
            std::function<void(ConnectionData<pT, pS>)> callback = [](ConnectionData<pT, pS>){}
        ) {
            
        }
};

int32_t main (
    int32_t argCount, 
    char* args[]
) {
    // sorry for the cheesy ascii art
    std::cout << "\n▄▀█ ▀▄▀ █▀█ █░░ █▀█ ▀█▀ █░░\n"
              <<   "█▀█ █░█ █▄█ █▄▄ █▄█ ░█░ █▄▄\n\n";

    cli::parseArgs(argCount, args);

    print::setLogLevel(print::logLevels::trace);

    // creating node instance
    Node myNode = Node(PUBLIC_KEY, LOCAL_PORT);

    // starting node instance
    myNode.start();

    // this thread checks for new packets in an infinite loop
    std::thread thr = std::thread([&](){
        while (1) {
            myNode.update(-1, true);
        }
    });

    if (REMOTE_IP != "" && REMOTE_PORT) {
        myNode.connect(
            REMOTE_IP,
            REMOTE_PORT
        );
    }

    while (1) {
        Packet<pT, pS> p;
        p.header.packetType = pT::textMessage;
        char s[256];
        std::cin.getline(s, 256);
        p << s;
        myNode.sendNode(REMOTE_PUBLIC_KEY, p);
    }
}