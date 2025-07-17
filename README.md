# Triton

Triton is the latest iteration of a 3D game framework that I have been writing off and on with various different technologies in some form or another for quite some time. The current implementation attempts to use as 'modern' of C++ as is possible and uses Vulkan for the graphics rendering. It is using vulkan_raii, which extends vulkan_hpp (which wraps the Vulkan Vulkan C api with object oriented C++ structures) and allows for the use of RAII concepts, which makes preventing resource leaks and ensuring resources are deallocated in the proper order less error prone, and enables a smoother integration with a programming style loosely inspired by OOP.

## Building/Running

The project is using CMake, and I use VSCode primarily for development. I primarily develop on linux using VSCode, but things should generally work on Windows, and almost on OS X.

Dependency management uses vanilla CMake using FetchContent but for now the Vulkan SDK still needs installed manually.

### Build Steps

1. Install the [Vulkan SDK](https://vulkan.lunarg.com/) and make sure the env var `VULKAN_SDK` gets set appropriately on your system
1. Install [CMake](https://cmake.org/download/).  Make sure `cmake` is on your path.
1. For Windows there is a CMake generator for Visual Studio 2022.
1. For Linux you can probably `sudo apt install llvm`.
1. For MacOS, I install the latest version of LLVM using homebrew.
1. Make sure `clang` and `clang++` are on your path.
1. Download [Ninja](https://github.com/ninja-build/ninja/releases). Make sure `ninja` is on your path.
1. Clone the repo
1. `cmake --preset debug`
1. `cmake --build --preset debug`

## Testing

Testing is using Catch2, Trompeloeil for mocking, llvm-cov for coverage, and still can be a bit rough.
There is a CMake Preset to use to enable coverage, `debug-coverage`.

1. `cmake --preset debug-coverage`
1. `cd build/debug-coverage`
1. `cmake --build . --target triton-tests`
1. `ctest`

This will write out an lcov info file into `build/debug-coverage/ccov` and you can configure an IDE plugin to display it in the gutters.

I created a target `triton-tests-coverage` and when running it with the debug-coverage preset it will produce an lcov file that you can hook up in an IDE. The file is written to `build/debug-coverage/ccov/triton-tests.info`

### CMake Presets

The project is using CMakePresets.json and has a few configure, build, and test presets.

#### Configure Presets

- debug - standard debug preset used mainly for local dev.
- debug-coverage - same as debug, but enables code coverage collection.
- release - creates an optimized release build

#### Build Presets

- debug - standard debug build
- msvc-debug - writes out a VS solution
- debug-coverage - debug build but with coverage enabled
- release - optimized release build

I mainly develop on linux using vscode and compiling with clang and linking against gnu libstdc++.

## OS X Notes

- OS X should mostly work, except drawIndirectCount wasn't supported on the hardware I had available, and I haven't circled back to add a code path that streams the count back from the culling shader.

Outdated instructions follow. I used to force the entire llvm toolset on all 3 platforms, but since have come around to realizing that using whatever is considered most native for the platform produces the least friction.

- For some consistency I `brew install llvm` and make sure `CC` and `CXX` env vars point to those compilers' executables
- have to set `MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS=1` in order for the executables to run on OS X.
- TODO: Figure out how to produce an app bundle with CPack so this env var can be bundled in there and you can just double click the app bundle.
- At present, everything should work on OS X. (except actually rendering anything)
