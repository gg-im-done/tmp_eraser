cmake_minimum_required(VERSION 3.24)

set(project_name "tmp_eraser")

project(${project_name} VERSION 1.69)

add_executable(${project_name})

set_target_properties(${project_name} PROPERTIES CXX_STANDARD 20)

target_sources(${project_name} PRIVATE tmp_eraser/tmp_eraser.cpp)

if(WIN32)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
