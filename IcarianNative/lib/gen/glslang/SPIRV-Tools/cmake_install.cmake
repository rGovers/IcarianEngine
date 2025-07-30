# Install script for directory: /home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/External/spirv-tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/external/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/source/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/tools/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/test/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/examples/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/spirv-tools" TYPE FILE FILES
    "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/External/spirv-tools/include/spirv-tools/libspirv.h"
    "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/External/spirv-tools/include/spirv-tools/libspirv.hpp"
    "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/External/spirv-tools/include/spirv-tools/optimizer.hpp"
    "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/External/spirv-tools/include/spirv-tools/linker.hpp"
    "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/External/spirv-tools/include/spirv-tools/instrument.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/pkgconfig" TYPE FILE FILES
    "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/SPIRV-Tools.pc"
    "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/SPIRV-Tools-shared.pc"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/River/repo/IcarianEditor/IcarianEngine/IcarianNative/lib/glslang/build/External/spirv-tools/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
