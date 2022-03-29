cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(APPEND CMAKE_CXX_FLAGS_RELEASE " -arch x86_64 -O3")
string(APPEND CMAKE_CC_FLAGS_RELEASE " -arch x86_64 -O3")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -arch x86_64 -O3")
string(APPEND CMAKE_CC_FLAGS_DEBUG " -arch x86_64 -O3")

project(asd VERSION 1.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/include/cryptopp/libcryptopp.a)

add_executable(axolotl src/main.cpp)
add_executable(test src/test.cpp)