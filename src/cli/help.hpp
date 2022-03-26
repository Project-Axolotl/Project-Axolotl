// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

namespace cli {
    void help () {
        std::vector<std::string> txt = {
            "Usage: ./axolotl -L <local_port> -I <remote_ip> -P <remote_port>",
            "",
            "-H, --help             Display this help and exit",
            "-L, --localport <int>  Port to listen on",
            "-I, --remoteip <addr>  Target server IP, defaults to 42069",
            "-P, --remoteport <int> Target server port, defaults to 42069",
            "-K, --publickey <str>  Public key of node for identification (4 chars)",
            "",
            "Examples:",
            "",
            "Start a node on port 4201:",
            "./axolotl -L 4201 -K asdf",
            "",
            "Start a node on port 4202, then connect to localhost:4201 and send an example message:",
            "./axolotl -L 4202 -K qwer -I localhost -P 4201",
            ""
        };
        for (std::string i : txt) std::cout << i << std::endl;
        exit(1);
    }
}