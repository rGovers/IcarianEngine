# IcarianEngine

![image](resources/Icarian_Logo_White.svg)

Icarian Engine is a cross platform game engine to allow ease of modding.

## Prerequisites
* Vulkan SDK
* Python3(SPIRV-Tools)
* zstd
### Linux
* GCC, Clang or Zig
### Windows
* MinGW

## Building

Building is done via CUBE.

The project can be built by running build.sh.

For extra options refer to --help on setup.sh.

Output is in the build folder.

Windows builds are done via cross compilation with MinGW.
MSVC is currently not supported.