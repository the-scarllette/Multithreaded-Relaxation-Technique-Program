cmake_minimum_required(VERSION 3.20)
project(ParallelCoursework1)

set(CMAKE_CXX_STANDARD 14)

add_executable(ParallelCoursework1 sequentialversion.c test.c parallelcode.c)
