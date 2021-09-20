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

set(SDL_Net ${CMAKE_SOURCE_DIR}/deps/SDL_net)

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
add_executable(Textlicious ${SOURCE_FILES} ${SDL_Net})

target_link_libraries(Textlicious raylib)
target_include_directories(Textlicious PRIVATE ${raylib_SOURCE_DIRS}/include ${CMAKE_SOURCE_DIR}/deps/SDL_net)