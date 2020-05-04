cmake_minimum_required(VERSION 3.15)
project(dlang)

set(CMAKE_CXX_STANDARD 17)

add_executable(dlang
        main.cpp
        src/dparser.cpp
        src/dparser.h
        src/dtransmt.cpp
        src/dtransmt.h
        src/ast.h
        src/ast.cpp
        src/dlexer.h
        src/dlexer.cpp
        src/stringref.h)


