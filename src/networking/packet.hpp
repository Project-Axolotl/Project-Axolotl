// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

template <typename pT, int* pS> struct PacketHeader {
    // packet type defines how to parse packet body
    pT packetType;
    // receiver has to reserve enough space to read the body
    size_t bodySize() const { 
        return pS[int(packetType)]; 
    };
};

template <typename pT, int* pS> struct Packet {
    PacketHeader<pT, pS> header;
    std::vector<uint8_t> body;

    // stream compatibility
    friend std::ostream& operator << (std::ostream& stream, Packet& packet) {
        stream << "type: " << int(packet.header.packetType) << " size: " << packet.header.bodySize();
        return stream;
    }

    // pushing data into the back of packet body
    template<typename dataT> friend Packet& operator << (Packet& packet, dataT& data) {
        static_assert(std::is_standard_layout<dataT>::value, "Data is too complex to be pushed into vector");
        // current size of vector will be the point we insert the data
        size_t i = packet.body.size();
        // make space for data in vector
        packet.body.resize(packet.body.size() + sizeof(dataT));
        // copy data into newly allocated vector space
        std::memcpy(packet.body.data() + i, &data, sizeof(dataT));
        return packet;
    }

    // pulling data from the back of packet body
    template<typename dataT> friend Packet& operator >> (Packet& packet, dataT& data) {
        static_assert(std::is_standard_layout<dataT>::value, "Data is too complex to be pushed into vector");
        // get location where the pulled data starts
        size_t i = packet.body.size() - sizeof(dataT);
        // copy data into variable
        std::memcpy(&data, packet.body.data() + i, sizeof(dataT));
        // shrink the vector by the size of pulled data
        packet.body.resize(i);
        return packet;
    }
};

template <typename pT, int* pS> class Connection;

// a packet with sender data attached for inner processing
template <typename pT, int* pS> struct MetaPacket {
    // pointer to the connection where the packet is from
    std::shared_ptr<Connection<pT, pS>> packetConn = nullptr;
    // public key of the sender node
    char senderPublicKey[5];
    // the original packet
    Packet<pT, pS> content;

    // operator override to be compatible with std::cout
    friend std::ostream& operator << (std::ostream& stream, MetaPacket& metaPacket) {
        stream << metaPacket.content << " from: \"" << metaPacket.senderPublicKey << "\"";
        return stream;
    }
};