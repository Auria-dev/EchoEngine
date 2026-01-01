@REM cmake -S . -B build && cmake --build build --target run

@REM winget install Ninja-build.Ninja
cmake -S . -B build -G "Ninja" && cmake --build build --target run