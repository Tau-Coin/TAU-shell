cmake_minimum_required(VERSION 3.10)
project(handler)

set(CMAKE_CXX_FLAGS "-std=c++17")

option(TORRENT_USE_OPENSSL "Use OPENSSL In libTAU" ON)

add_definitions(-D TORRENT_USE_OPENSSL)

add_definitions(-D TORRENT_USE_LIBCRYPTO)

include_directories(../../include)
add_library(handler "")

target_link_libraries(handler TAU)

target_link_libraries(handler dl)

#target_link_libraries(handler iconv)

#link sqlite lib
target_link_libraries(handler sqlite3)

target_link_libraries(handler rpc)

target_link_libraries(handler pthread)

target_sources(handler 
  PRIVATE
	alert_handler
	tau_handler
)
