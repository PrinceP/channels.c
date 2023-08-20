cmake_minimum_required(VERSION 3.16..3.24)
project(chan VERSION 0.0.1
                    DESCRIPTION "Channels of Go in cpp"
                    LANGUAGES CXX)

add_definitions(-std=c++17)
add_executable(chan main.cpp channels.cpp)

if(UNIX)
add_definitions(-O2 -pthread)
endif(UNIX)
