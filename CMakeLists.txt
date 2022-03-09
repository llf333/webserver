cmake_minimum_required(VERSION 3.16.3)

project(WEBSERVER)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

aux_source_directory(./src DIR_SRC)
include_directories(./include)

add_executable(Demo ${DIR_SRC})