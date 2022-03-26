// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

template <typename pT, int* pS> struct ConnectionData {
    ::ipAddress ipAddress;
    ::port port;
    std::shared_ptr<::Connection<pT, pS>> connection;
};

struct PublicKeyCompare {
    bool operator() (const char* x, const char* y) const {
        return std::strcmp(x, y);
    }
};

template <typename pT, int* pS> class RoutingTable {
    private:
        std::mutex containerMutex;
        std::map<
            publicKey,
            ConnectionData<pT, pS>,
            PublicKeyCompare
        > container;
        std::condition_variable blockingCV;
        std::mutex blockingMutex;

    public:
        RoutingTable() = default;
        RoutingTable(const RoutingTable&) = delete;
        virtual ~RoutingTable() { clear(); }

        // basic methods encapsulating an std::map

        bool empty() {
            std::scoped_lock lock(containerMutex);
            return container.empty();
        }

        size_t count() {
            std::scoped_lock lock(containerMutex);
            return container.size();
        }

        void clear() {
            std::scoped_lock lock(containerMutex);
            container.clear();
        }

        void set(
            publicKey _publicKey,
            ipAddress _ipAddress,
            port _port,
            std::shared_ptr<Connection<pT, pS>> _conn = nullptr
        ) {
            std::scoped_lock lock(containerMutex);
            ConnectionData<pT, pS> node;
            node.ipAddress = _ipAddress;
            node.port = _port;
            node.connection = _conn;
            container.insert(std::pair<publicKey, ConnectionData<pT, pS>>(
                _publicKey,
                node
            ));
        }

        ConnectionData<pT, pS>* get(
            publicKey _publicKey
        ) {
            std::scoped_lock lock(containerMutex);
            auto res = container.find(_publicKey);
            if (res == container.end()) {
                return nullptr;
            }
            return &(res->second);
        }
};