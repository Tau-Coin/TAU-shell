cmake_minimum_required(VERSION 3.10)
project(TAU-shell)

set(CMAKE_CXX_FLAGS "-std=c++17")

option(TORRENT_USE_OPENSSL "Use OPENSSL In libTAU" ON)

add_definitions(-D TORRENT_USE_OPENSSL)

add_definitions(-D TORRENT_USE_LIBCRYPTO)

include_directories($ENV{OPENSSL_ROOT}/include $ENV{BOOST_ROOT} $ENV{SQLITE_ROOT}/include "/usr/local/include")

link_directories($ENV{BOOST_ROOT}/lib $ENV{OPENSSL_ROOT}/lib $ENV{SQLITE_ROOT}/lib "/usr/local/lib")

include_directories(include)

add_executable(main src/main.cpp)

#link tau lib
target_link_libraries(main TAU)

#sub rpc
add_subdirectory(src/util)
target_link_libraries(main util)

#sub rpc
add_subdirectory(src/rpc)
target_link_libraries(main rpc)

#sub handler
add_subdirectory(src/handler)
target_link_libraries(main handler)
