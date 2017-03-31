cmake_minimum_required(VERSION 3.6)
project(new_ping)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        util.cc
        ping.cc
        main.cc
        )

add_compile_options(-Wall -Werror -g)

link_libraries(pthread)

add_executable(zy_ping ${SOURCE_FILES})