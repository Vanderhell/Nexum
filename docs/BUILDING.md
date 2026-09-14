# Building Nexum

Nexum is C11 and uses CMake. The project currently declares CMake 3.16 as its
minimum in `CMakeLists.txt`; use a compiler and build tool supported by your CMake
installation. Out-of-source builds are required for clean repository hygiene.

## Linux with GCC

```sh
cmake -S . -B build-gcc-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc
cmake --build build-gcc-debug

cmake -S . -B build-gcc-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc
cmake --build build-gcc-release
```

## Linux with Clang

```sh
cmake -S . -B build-clang-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build-clang-debug

cmake -S . -B build-clang-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang
cmake --build build-clang-release
```

## Windows with MSVC

Run from an x64 Native Tools Command Prompt for Visual Studio:

```bat
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64
cmake --build build-msvc --config Debug
cmake --build build-msvc --config Release
```

The generator name may differ with the installed Visual Studio release. CMake's
multi-configuration generator uses `--config`; single-configuration Ninja builds
use `CMAKE_BUILD_TYPE`.

## Warnings

The library and tests build with `/W4` on MSVC and with
`-Wall -Wextra -Wpedantic -Wconversion -Wshadow` on GCC/Clang. Project warnings are
expected to remain at zero.
