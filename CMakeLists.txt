cmake_minimum_required(VERSION 3.20)
project(Textlicious C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

include(FetchContent)

FetchContent_Declare(
        RAYLIB
        GIT_REPOSITORY "https://github.com/raysan5/raylib"
        GIT_TAG 3.0.0
)

FetchContent_GetProperties(RAYLIB)
if (NOT raylib_POPULATED)
    FetchContent_Populate(RAYLIB)
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
endif ()
FetchContent_Declare(
        NNG
        GIT_REPOSITORY "https://github.com/nanomsg/nng"
        GIT_TAG v1.5.2
)

FetchContent_GetProperties(NNG)
if (NOT nng_POPULATED)
    FetchContent_Populate(NNG)
    add_subdirectory(${nng_SOURCE_DIR} ${nng_BINARY_DIR})
endif()

set(SOURCE_FILES
        termInput.c
        list.c
        list.h
        packedSet.c
        packedSet.h
        source.c
        pool.c
        pool.h ecs.c ecs.h
        termInput.h)
add_executable(Textlicious ${SOURCE_FILES})
target_link_libraries(Textlicious raylib nng)
target_include_directories(Textlicious PRIVATE ${raylib_SOURCE_DIRS}/include ${nng_SOURCE_DIRS}/include)