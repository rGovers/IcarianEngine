cmake_minimum_required(VERSION 3.0)

add_subdirectory("IcarianNative/")

project(IcarianCS VERSION 0.1 LANGUAGES CSharp)

include(CSharpUtilities)

file(GLOB_RECURSE SOURCES "IcarianCS/src/*.cs")

add_executable(IcarianCS ${SOURCES})

set_target_properties(IcarianCS PROPERTIES ENABLE_EXPORTS 1)
set_target_properties(IcarianCS PROPERTIES VS_DOTNET_REFERENCES "System.Xml")
set_target_properties(IcarianCS PROPERTIES VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6")
set_target_properties(IcarianCS PROPERTIES WIN32_EXECUTABLE FALSE)
set_target_properties(IcarianCS
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/"
)
add_custom_command(TARGET IcarianCS
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/bin/IcarianCS.exe"
        "${CMAKE_SOURCE_DIR}/bin/IcarianCS.dll")