cmake_minimum_required(VERSION 3.10)
project(liquibook)

option(QUICKFAST_SRC "Set QUICKFAST_SRC variable with path to QUICKFAST Source")
if(NOT QUICKFAST_SRC)
    message( SEND_ERROR "Set QUICKFAST_SRC variable with path to QUICKFAST Source" )
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(QUICKFAST_BUILD ${QUICKFAST_SRC}/cmake-build-debug)
    add_compile_definitions(_DEBUG)
else()
    set(QUICKFAST_BUILD ${QUICKFAST_SRC}/cmake-build-release)
endif()

add_subdirectory(src)
add_subdirectory(examples)