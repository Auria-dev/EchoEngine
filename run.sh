# cmake -S . -B build; cmake --build build --target run

cmake -S . -B build -G Ninja && cmake --build build --target run