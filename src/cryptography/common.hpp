// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once

#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>

#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/rijndael.h>
#include <cryptopp/rsa.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/queue.h>
#include <cryptopp/gcm.h>
#include <cryptopp/pssr.h>

using namespace cli;

#include "sha.hpp"
#include "aes.hpp"
#include "rsa.hpp"
#include "cracking.hpp"