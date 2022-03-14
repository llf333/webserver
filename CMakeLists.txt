cmake_minimum_required(VERSION 3.16.3)

project(WEBSERVER)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")
set(CMAKE_CXX_STANDARD 17)

aux_source_directory(./src DIR_SRC)
include_directories(./include)

add_executable(WEBSERVER ${DIR_SRC} )

include(FetchContent)
FetchContent_Declare(spdlog #库名字
        GIT_REPOSITORY git@github.com:gabime/spdlog.git #仓库地址，这里采用的是ssh版本
        GIT_TAG v1.x
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/spdlog
        )
FetchContent_MakeAvailable(spdlog)

# 项目中使用spdlog
target_link_libraries(WEBSERVER PRIVATE spdlog::spdlog)