cmake_minimum_required(VERSION 3.22)

if(WIN32)
    message(STATUS "Using Windows platform source")
    add_subdirectory ("Windows")
else(WIN32)
    message(STATUS "Using POSIX platform source")
    add_subdirectory ("Posix")
endif(WIN32)