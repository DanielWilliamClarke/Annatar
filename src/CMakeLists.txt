﻿#change this if you need to target a specific CMake version
cmake_minimum_required(VERSION 3.8)

# Enable debug symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

project(Annatar)

## If you want to link SFML statically
set(SFML_STATIC_LIBRARIES TRUE)
## In most cases better set in the CMake cache
set(SFML_DIR "../libraries/SFML-2.5.1/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

# Set version information in a config.h file
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/components")
include_directories("${PROJECT_SOURCE_DIR}/entity")
include_directories("${PROJECT_SOURCE_DIR}/factories")

include_directories("${PROJECT_BINARY_DIR}")

# Define sources and executable
set(EXECUTABLE_NAME "Annatar")
add_executable(${EXECUTABLE_NAME}
 main.cc
 game.cc
 entity/entity.cc
 entity/entity_composition.cc
 entity/entity_object.cc
 player.cc
 player_input.cc
 fps.cc 
 animation.cc
 components/animation_component.cc
 components/movement_component.cc
 components/hitbox_component.cc
)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

# CPack packaging
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
include(CPack)

target_link_libraries(Annatar sfml-graphics sfml-audio)

add_custom_command(TARGET Annatar POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/src/assets $<TARGET_FILE_DIR:Annatar>/assets)