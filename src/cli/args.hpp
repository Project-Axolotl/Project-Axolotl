// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

uint16_t LOCAL_PORT{};
uint16_t REMOTE_PORT{};
char PUBLIC_KEY[5]{};
std::string REMOTE_IP{};
char REMOTE_PUBLIC_KEY[5]{};

namespace cli {
    void parseArgs(int32_t argCount, char* args[]) {
        // running the program without argument should always fail
        if (argCount < 2) {
            cli::help();
        }
        // parsing arguments
        for (int i = 1; i < argCount; i++) {
            if (!std::strcmp(args[i], "--remoteip") || !std::strcmp(args[i], "-I")) {
                if (i + 1 >= argCount || args[i + 1][0] == '-') cli::help();
                REMOTE_IP = args[i + 1];
                i++;
            }
            else if (!std::strcmp(args[i], "--remoteport") || !std::strcmp(args[i], "-P")) {
                if (i + 1 >= argCount || args[i + 1][0] == '-') cli::help();
                REMOTE_PORT = atoi(args[i + 1]);
                i++;
            }
            else if (!std::strcmp(args[i], "--localport") || !std::strcmp(args[i], "-L")) {
                if (i + 1 >= argCount || args[i + 1][0] == '-') cli::help();
                LOCAL_PORT = atoi(args[i + 1]);
                i++;
            }
            else if (!std::strcmp(args[i], "--publickey") || !std::strcmp(args[i], "-K")) {
                if (i + 1 >= argCount || args[i + 1][0] == '-') cli::help();
                std::strcpy(PUBLIC_KEY, args[i + 1]);
                i++;
            }
            else if (!std::strcmp(args[i], "--help") || !std::strcmp(args[i], "-H")) {
                cli::help();
            }
            // if an argument can not be processed
            else { 
                cli::help();
                exit(1);
            }
        }
    }
}