@echo off
echo Building yaml-cpp with Qt MinGW compiler...

rem Clean old builds
if exist "external\yaml-cpp\build" rmdir /s /q "external\yaml-cpp\build"
if exist "build\external\yaml-cpp-win" rmdir /s /q "build\external\yaml-cpp-win"

rem Create proper build directory
mkdir "build\external\yaml-cpp-win"
cd "build\external\yaml-cpp-win"

rem Set environment for Qt MinGW
set PATH=C:\Qt\Tools\mingw1310_64\bin;%PATH%
set CMAKE_PREFIX_PATH=C:\Qt\6.8.0\mingw_64

rem Configure with MinGW
cmake .. -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DYAML_CPP_BUILD_TESTS=OFF ^
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON ^
    -DCMAKE_CXX_STANDARD=17

rem Build
mingw32-make -j4

echo Build completed!
pause