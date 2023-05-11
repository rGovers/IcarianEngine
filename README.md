# IcarianEngine
Icarian Engine is a cross platform game engine to allow ease of modding.

## Prerequisites
* cmake
* Mono
* Vulkan SDK

## Building

### Windows
Requires cygwin for compiling Mono. 
Refer to the following for requirements https://www.mono-project.com/docs/compiling-mono/windows/ 

Enter the mono directory in deps/flare-mono/ and run the following commands to build mono 
```
./autogen.sh
./configure --prefix=$PWD/build --host=x86_64-w64-mingw32 --disable-boehm --enable-msvc
make -j6
make install
```
After which run in cmd in the root directory
```
./setup.bat
```
Output will be in build.
Open the generated sln file and build all projects.
### Linux
Run the following scripts
```
./setup.sh
./buildDebug.sh
```
Output will be in build