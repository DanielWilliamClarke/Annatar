﻿cmake_minimum_required (VERSION 3.8)

project(Annatar CXX)

include(dependencies.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Enable debug symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE EQUAL Debug) 
  set(SFML_GENERATE_PDB TRUE)
endif()

# Add src
include_directories(src)
add_subdirectory (src)