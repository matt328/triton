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

- [ ] stick to clean and orderly object oriented design principles
- [ ] utilize as much modern C++ as possible
- [ ] effectively use an ECS
- [x] set up vulkan's debug layers with naming for all objects
- [x] draw a triangle
- [x] load models
- [x] configurable input mapping system
- [x] debug UI with ImGui
- [ ] research Taskflow multithreading and framegraph concepts
- [ ] split project into multiple modules, base library, editor, game
- [ ] research using c++ for 'scripting' and being able to recompile parts of the game without having to stop the executable
- [ ] draw a bunch of models effectively
- [ ] implement deferred rendering
- [ ] implement PBR materials
- [ ] start building some kind of game

## Building/Running

This has recently been migrated to using CMake, and I use vscode primarily for development. So far everything works well on Windows ~~as well as OS X and linux~~.

While my goal has been to leverage vcpkg and not have to struggle with dependencies, I'm still just installing the Vulkan SDK and setting the VULKAN_SDK env var and relying on CMake's `find_package()` to do the thing. This area admittedly needs a little more research and understanding to make locating the Vulkan headers and libraries a bit more consistent across machines and platforms.

1. Clone the repo
2. Init vcpkg:

   ```PowerShell
   git submodule init
   git submodule update
   .\vcpkg\bootstrap-vcpkg.(bat|sh)
   ```

3. `cmake -B build . -G "Ninja"`
4. `cmake --build .\build --config Debug -j 18 --target triton`
5. `./bin/triton[.exe]`

## C++ Compiler Notes

Largely simplified thanks to using Ninja instead of make.  Ninja is cross platform, so no messing around with mingw32 builds, and seems to be a bit faster even with this tiny little project. Just install CMake, LLVM, and Ninja. These should be the same on all 3 platforms now.

Install [CMake](https://cmake.org/download/).  Make sure `cmake` is on your path.

For Windows, install [LLVM](https://github.com/llvm/llvm-project/releases/)

For Linux you can probably `sudo apt install llvm`.

Make sure `clang` and `clang++` are on your path.

Download [Ninja](https://github.com/ninja-build/ninja/releases). Make sure `ninja` is on your path.

You can set `CC` and `CCX` env vars to `path\to\clang.exe` and `path\to\clang++.exe` if you don't want to risk VSCode detecting the wrong 'kit'. This isn't a big deal, you just have to use the quick menu and select `CMake: Select a Kit` and select the kit that will use clang and clang++ you installed earlier.

The clangd extension gives a much better experience in VSCode than Microsoft's Intellisense. Once you get it configured correctly. Include paths are kind of wonky. See the `.clangd` file at the root of the project.
