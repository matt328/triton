# Triton

Triton is the latest of a 3D game framework that I have been writing off and on with various different technologies
in some form or another for the better part of 20 years now. The current implementation attempts to use as
'modern' of C++ as is possible and uses Vulkan for the graphics rendering. As such I'm using vulkan_raii, which
extends vulkan_hpp (which wraps the Vulkan Vulkan C api with object oriented C++ structures) and allows for the
use of RAII concepts, removing alot of the difficulty in preventing resource leaks and tearing everything down
in the correct order.

At present it only loads up a sample gltf file and renders it fully textured, and since accomplishing that milestone,
I have been taking some time to refactor this brute force prototype code into a solid maintainable and extendable base.

## Roadmap

- stick to clean and orderly object oriented design principles
- utilize as much modern C++ as possible
- set up vulkan's debug layers with naming for all objects
- draw a triangle
- load models
- draw a bunch of models
- draw a bunch of models with different renderpasses and pipelines
- add profiling
- draw a bunch of models effectively
- create an actual game

## Building/Running

This has recently been migrated to using CMake, and I use vscode primarily for development. So far everything works well on Windows as well as OS X. The only generators I've worked with and can verify are working are `MinGW Makefiles` on Windows and `Unix Makefiles` on OS X. I also use clang >= 15. I _think_ msvc will compile this, but I honestly doubt gcc will.
While my goal has been to leverage vcpkg and not have to struggle with dependencies, I'm still just installing the Vulkan SDK and setting the VULKAN_SDK env var and relying on CMake's `find_package()` to do the thing.

1. Clone the repo
2. Init vcpkg:

```PowerShell
git submodule init
git submodule update
.\vcpkg\bootstrap-vcpkg.(bat|sh)
```

3. `cmake -B build .`
4. `cmake --build .\build --config Debug -j 18 --target driver`
5. `./bin/driver[.exe]`

## Windows Toolchain Notes

This took me forever to rediscover. Uses LLVM, clang, lld and lldb.

Install CMake by downloading the windows installer.

Install LLVM using the installer from the github release page https://github.com/llvm/llvm-project/releases/

Download some dude's MinGW build using LLVM from here https://github.com/mstorsjo/llvm-mingw/releases

Extract it somewhere and add its `/bin` folder to your PATH

Deal with CMake not supporting some feature of C++ that the latest version of LLVM causes it to opt
into anyway.
