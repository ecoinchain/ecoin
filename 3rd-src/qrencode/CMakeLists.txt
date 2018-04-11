cmake_minimum_required(VERSION 3.2)
project(qrencode)
find_package(Threads)

add_definitions(-D__STATIC=static)

add_library(qrencode STATIC qrencode.c qrencode_inner.h 
						 qrinput.c qrinput.h 
						 bitstream.c bitstream.h 
						 qrspec.c qrspec.h 
						 rscode.c rscode.h 
						 split.c split.h 
						 mask.c mask.h 
						 mqrspec.c mqrspec.h 
						 mmask.c mmask.h
)

target_include_directories(qrencode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(qrencode PROPERTIES FOLDER "third_party/")
