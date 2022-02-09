cmake_minimum_required(VERSION 3.10)
project(util)

set(CMAKE_CXX_FLAGS "-std=c++17")

option(TORRENT_USE_OPENSSL "Use OPENSSL In libTAU" ON)

add_definitions(-D TORRENT_USE_OPENSSL)

add_definitions(-D TORRENT_USE_LIBCRYPTO)

include_directories(../../include)
add_library(util "")

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
