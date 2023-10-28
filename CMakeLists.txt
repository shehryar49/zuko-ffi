cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)
project(ffi)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(ffi SHARED ffi.cpp ffi.h)

if(UNIX)
  target_link_libraries(ffi ${CMAKE_DL_LIBS})
endif()
target_link_libraries(ffi -lffi)


