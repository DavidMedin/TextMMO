cmake_minimum_required(VERSION 3.16)
project(Textlicious C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

include(FetchContent)

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
        termInput.h
        vec.c
        vec.h
        server.c
        server.h
        source.h
        deps/logger/log.c
        Content/humanoid.c Content/humanoid.h Content/lookable.c Content/lookable.h Content/item.c Content/item.h)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_USE_COLOR")
add_executable(Textlicious ${SOURCE_FILES})
target_link_libraries(Textlicious nng m)
target_include_directories(Textlicious PRIVATE ${nng_SOURCE_DIRS}/include deps/logger)