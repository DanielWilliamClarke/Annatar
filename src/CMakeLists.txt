﻿cmake_minimum_required (VERSION 3.8)

set(BINARY ${CMAKE_PROJECT_NAME})
set(BINARY_LIB ${CMAKE_PROJECT_NAME}_lib)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true
    *.h
    *.cc
)
set(SOURCES
    ${SOURCES}
)
set(EXECUTABLE_NAME ${BINARY})

# Create executable to run game
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${BINARY} PUBLIC ImGui-SFML::ImGui-SFML range-v3)

# Create lib for testing purposes
add_library(${BINARY_LIB} STATIC ${SOURCES})
target_link_libraries(${BINARY_LIB} PUBLIC ImGui-SFML::ImGui-SFML range-v3)

# Copy assets on post build
add_custom_command(TARGET ${BINARY} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/assets $<TARGET_FILE_DIR:${BINARY}>/assets
)