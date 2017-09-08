cmake_minimum_required(VERSION 3.6)
project(loc_me)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    location/geohash.cpp
    location/geohash.h
    CMakeLists.txt
    main.cpp
    README.md)

add_executable(loc_me ${SOURCE_FILES})