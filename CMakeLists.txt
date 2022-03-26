cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(APPEND CMAKE_CXX_FLAGS_RELEASE " -arch x86_64 -O3")
string(APPEND CMAKE_CC_FLAGS_RELEASE " -arch x86_64 -O3")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -arch x86_64 -O3")
string(APPEND CMAKE_CC_FLAGS_DEBUG " -arch x86_64 -O3")

project(axolotl VERSION 1.0)


add_executable(axolotl src/main.cpp)

target_include_directories(axolotl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(axolotl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/cryptopp/libcryptopp.a)


add_executable(test src/test.cpp)

target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/cryptopp/libcryptopp.a)