cmake_minimum_required(VERSION 3.10)

###
# Project definition
###
project(isotp LANGUAGES C)

###
# Get all include directories
###
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/
)


if(NOT CMAKE_CROSSCOMPILING)
    enable_testing()
    add_executable( buffer_pack_unpack_16_test
                    buffer_pack_unpack_16.c
                    test.c )

    add_test( NAME buffer_pack_unpack_16_test 
              COMMAND buffer_pack_unpack_16_test )
endif()



