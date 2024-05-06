cmake_minimum_required(VERSION 3.13)
project(trimesh_color)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

include_directories(/Users/sallo/Documents/University/Courses/3D_Geometric_Modelling_and_Processing/exercises/_vcglib)
include_directories(/Users/sallo/Documents/University/Courses/3D_Geometric_Modelling_and_Processing/exercises/_vcglib/vcg)
include_directories(/Users/sallo/Documents/University/Courses/3D_Geometric_Modelling_and_Processing/exercises/_vcglib/eigenlib)
include_directories(/Users/sallo/Documents/University/Courses/3D_Geometric_Modelling_and_Processing/exercises/_vcglib/eigenlib/Eigen)
include_directories(/Users/sallo/Documents/University/Courses/3D_Geometric_Modelling_and_Processing/exercises/_vcglib/eigenlib/unsupported)

set(SOURCES
	trimesh_color.cpp
	/Users/sallo/Documents/University/Courses/3D_Geometric_Modelling_and_Processing/exercises/_vcglib/wrap/ply/plylib.cpp)

add_executable(trimesh_color
	${SOURCES})

