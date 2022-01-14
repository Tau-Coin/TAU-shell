cmake_minimum_required(VERSION 3.10)
project(handler)

set(CMAKE_CXX_FLAGS "-std=c++17")

option(TORRENT_USE_OPENSSL "Use OPENSSL In libTAU" ON)

add_definitions(-D TORRENT_USE_OPENSSL)

add_definitions(-D TORRENT_USE_LIBCRYPTO)

include_directories(../../include)
add_library(handler "")

#link tau lib

include_directories("/home/cowtc/data/libTAU-build/openssl/include" "/home/cowtc/data/libTAU-build/boost" "/home/cowtc/data/libTAU-build/sqlite/include" "/usr/local/include")

link_directories("/home/cowtc/data/libTAU-build/boost/lib" "/home/cowtc/data/libTAU-build/openssl/lib" "/home/cowtc/data/libTAU-build/sqlite/lib" "/usr/local/lib")

target_link_libraries(handler TAU)

target_link_libraries(handler dl)

#target_link_libraries(handler iconv)

#link sqlite lib
target_link_libraries(handler sqlite3)

target_link_libraries(handler rpc)

target_link_libraries(handler pthread)

target_sources(handler 
  PRIVATE
	tau_handler
	db_util	
	hex_util	
	escape_json	
)
