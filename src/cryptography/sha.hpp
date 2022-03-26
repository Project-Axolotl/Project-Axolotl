// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

namespace SHA {
    std::string Hash(
        std::string input
    ) {
        std::string digest;
        CryptoPP::SHA256 hash;

        CryptoPP::StringSource source(
            input, 
            true,
            new CryptoPP::HashFilter(
                hash,
                new CryptoPP::Base64Encoder (
                    new CryptoPP::StringSink(digest)
                )
            )
        );

        return digest;
    }
}