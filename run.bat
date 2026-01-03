@REM cmake -S . -B build && cmake --build build --target run

@REM winget install Ninja-build.Ninja

@echo off
setlocal

if exist build\CMakeCache.txt (
    findstr /C:"CMAKE_GENERATOR:INTERNAL=Ninja" build\CMakeCache.txt >nul
    if errorlevel 1 (rmdir /s /q build)
)

if not exist build (cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug)
cmake --build build --target run

endlocal