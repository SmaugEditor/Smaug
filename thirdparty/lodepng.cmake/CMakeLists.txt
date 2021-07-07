cmake_minimum_required(VERSION 3.0)

project(lodepng)

add_library(lodepng STATIC ${CMAKE_CURRENT_LIST_DIR}/lodepng/lodepng.cpp ${CMAKE_CURRENT_LIST_DIR}/lodepng/lodepng.h )
target_include_directories(lodepng PUBLIC ${CMAKE_CURRENT_LIST_DIR}/lodepng)
