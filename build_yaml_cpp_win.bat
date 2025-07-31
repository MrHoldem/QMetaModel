@echo off
REM Batch script to build yaml-cpp for Windows MinGW

echo ===============================================
echo Building yaml-cpp for Windows MinGW
echo ===============================================

REM Change to project root
cd /d "%~dp0"

echo Cleaning old build files...

REM Clean old build files in yaml-cpp source directory
if exist "external\yaml-cpp\build" (
    echo Removing external\yaml-cpp\build...
    rmdir /s /q "external\yaml-cpp\build"
)

if exist "external\yaml-cpp\CMakeCache.txt" (
    echo Removing external\yaml-cpp\CMakeCache.txt...
    del "external\yaml-cpp\CMakeCache.txt"
)

REM Create and clean our build directory
if exist "build\external\yaml-cpp-win" (
    echo Removing old build\external\yaml-cpp-win...
    rmdir /s /q "build\external\yaml-cpp-win"
)

mkdir "build\external\yaml-cpp-win"

REM Go to build directory
cd "build\external\yaml-cpp-win"

echo Configuring yaml-cpp with CMake...

REM Configure with CMake using MinGW
cmake -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DYAML_CPP_BUILD_TESTS=OFF ^
    -DYAML_CPP_BUILD_TOOLS=OFF ^
    -DYAML_CPP_BUILD_CONTRIB=OFF ^
    -DCMAKE_INSTALL_PREFIX=install ^
    "..\..\..\external\yaml-cpp"

if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed!
    echo.
    echo Make sure you have:
    echo - CMake installed and in PATH
    echo - MinGW/MSYS2 installed and in PATH
    echo - Run this from Qt Creator command prompt or set up environment
    pause
    exit /b 1
)

echo.
echo Building yaml-cpp...

REM Build with mingw32-make
mingw32-make -j4

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed!
    echo.
    echo Check the error messages above.
    pause
    exit /b 1
)

echo.
echo ===============================================
echo yaml-cpp built successfully!
echo Library location: %CD%\libyaml-cpp.a
echo ===============================================

REM Go back to project root
cd ..\..\..

echo.
echo Now you can build the main QMetaModel project in Qt Creator.
echo.
pause