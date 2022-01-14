cmake_minimum_required(VERSION 3.10)
project(TAU-shell)

set(CMAKE_CXX_FLAGS "-std=c++17")

option(TORRENT_USE_OPENSSL "Use OPENSSL In libTAU" ON)

add_definitions(-D TORRENT_USE_OPENSSL)

add_definitions(-D TORRENT_USE_LIBCRYPTO)

include_directories("/home/cowtc/data/libTAU-build/openssl/include" "/home/cowtc/data/libTAU-build/boost" "/home/cowtc/data/libTAU-build/sqlite/include" "/usr/local/include")

link_directories("/home/cowtc/data/libTAU-build/boost/lib" "/home/cowtc/data/libTAU-build/openssl/lib" "/home/cowtc/data/libTAU-build/sqlite/lib" "/usr/local/lib")

include_directories(include)

add_executable(main src/main.cpp)

#link tau lib
target_link_libraries(main TAU)

#sub rpc
add_subdirectory(src/rpc)
target_link_libraries(main rpc)

#sub handler
add_subdirectory(src/handler)
target_link_libraries(main handler)
