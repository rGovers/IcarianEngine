cmake_minimum_required (VERSION 3.8)

project (FlareBase VERSION 0.1 LANGUAGES C CXX)

option(ENABLE_ASSERT "Enables flare asserts" OFF)

set(CMAKE_CXX_STANDARD 17)

set (FLAREBASE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set (FLAREBASE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})

configure_file(
    "${PROJECT_SOURCE_DIR}/FlareBaseConfig.h.in"
    "${PROJECT_SOURCE_DIR}/include/FlareBaseConfig.h"
)

if (ENABLE_ASSERT)
    add_compile_definitions(FLARE_ENABLE_ASSERT)
endif()

add_compile_definitions(GLM_FORCE_QUAT_DATA_XYZW)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_FORCE_RADIANS)

include_directories("${PROJECT_SOURCE_DIR}/include/"
                    "${PROJECT_SOURCE_DIR}/../deps/flare-glm/"
                    "${PROJECT_SOURCE_DIR}/../deps/flare-tinyxml2/")

file(GLOB SOURCES "src/*.cpp" "../deps/flare-tinyxml2/tinyxml2.cpp")

add_library(FlareBase ${SOURCES})