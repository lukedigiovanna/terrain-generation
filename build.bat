@echo off

REM Check if build directory exists, create it if not
if not exist "build" (
    mkdir build
)

cd build

del evolution.exe
cmake -G "MinGW Makefiles" ..
make
