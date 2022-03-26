// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <unistd.h>
#include <bitset>
#include <cstring>
#include <thread>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <map>
#include <array>
#include <unordered_map>

//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::asio;
using namespace cli;

#include "../cli/print.hpp"

typedef std::string ipAddress;
typedef uint16_t port;
typedef char* publicKey;

#include "packet.hpp"
#include "queue.hpp"
#include "routingTable.hpp"
#include "connection.hpp"
#include "endpoint.hpp"
