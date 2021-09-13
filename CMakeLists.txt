cmake_minimum_required(VERSION 3.20)
project(Textlicious C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

set(SOURCE_FILES
        list.c
        list.h
        packedSet.c
        packedSet.h
        source.c
        pool.c
        pool.h ecs.c ecs.h
        input.h
        input.c)
add_executable(Textlicious ${SOURCE_FILES})

