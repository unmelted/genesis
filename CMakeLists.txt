## 2021.09.16 
## created by Kelly (ekma@4dreplay.com)
##

cmake_minimum_required(VERSION 3.7)
#set(CMAKE_C_COMPILER "clang")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang")

project(Genesis)
set (PLATFROM MAC) # available : WIN, ARM, LINUX
SET(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wno-invalid-source-encoding -Wno-deprecated-register -Wno-writable-strings -Wno-parentheses -O2 -g")
#set(LIB_INCLUDE_DIR "/usr/local/include")


set(PRJ_PATH        ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB PRJ_SRC      ${PRJ_PATH}/src/*.cpp
                            ${PRJ_PATH}/src/detector/*.cpp
                            ${PRJ_PATH}/util/*.cpp
                            ${PRJ_PATH}/genesis.cpp)

file(GLOB PRJ_INC      ${PRJ_PATH}/src/*.hpp
                            ${PRJ_PATH}/src/detector/*.hpp
                            ${PRJ_PATH}/util/*.hpp
                            ${PRJ_PATH}/include/*.hpp
                            )


include_directories(${PRJ_INC})
message(${PRJ_INC})
add_library(genesis SHARED ${PRJ_SRC})
target_link_libraries(genesis)

add_executable(runtest ${PRJ_SRC})
target_link_libraries(runtest)