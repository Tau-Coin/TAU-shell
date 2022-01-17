cmake_minimum_required(VERSION 3.10)
project(rpc)
include_directories(../../include)
add_library(rpc "")
target_link_libraries(rpc dl)
target_link_libraries(rpc pthread)
target_sources(rpc 
  PRIVATE
	mongoose
	json_util
	jsmn
	cdecode
	base64
	webui
	auth
)
