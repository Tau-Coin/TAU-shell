cmake_minimum_required(VERSION 3.10)
project(util)

set(CMAKE_CXX_FLAGS "-std=c++17")

option(TORRENT_USE_OPENSSL "Use OPENSSL In libTAU" ON)

add_definitions(-D TORRENT_USE_OPENSSL)

add_definitions(-D TORRENT_USE_LIBCRYPTO)

include_directories(../../include)
add_library(util "")

#link tau lib

include_directories("/home/cowtc/data/libTAU-build/openssl/include" "/home/cowtc/data/libTAU-build/boost" "/home/cowtc/data/libTAU-build/sqlite/include" "/usr/local/include")

link_directories("/home/cowtc/data/libTAU-build/boost/lib" "/home/cowtc/data/libTAU-build/openssl/lib" "/home/cowtc/data/libTAU-build/sqlite/lib" "/usr/local/lib")

target_link_libraries(util TAU)

target_link_libraries(util dl)

#target_link_libraries(util iconv)

#link sqlite lib
target_link_libraries(util sqlite3)

target_link_libraries(util rpc)

target_link_libraries(util pthread)

target_sources(util 
  PRIVATE
	cdecode	
	base64
	json_util	
	db_util	
	hex_util	
	escape_json	
)
