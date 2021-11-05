cmake_minimum_required(VERSION 3.20)
project(ParallelCoursework1)

set(CMAKE_CXX_STANDARD 14)


set(SOURCE_FILES lab4_v2.c)

add_executable(ParallelCoursework1 sequentialversion.c test.c parallelcode.c)

