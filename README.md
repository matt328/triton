# Updates:

Original Triton has reached the point where some decisions that enabled faster prototyping and development now need to be reconsidered. vulkan_raii helped with learning Vulkan's concepts without being bogged down so much in the verbosity of the C API, but creating my own thin wrappers over the Vulkan components' handles will provide a better degree of control and understanding. Boost::DI was also great for prototyping, but as the project grew, I feel into the trap of foregoing solid architecture design in favor of a component soup, where anything and everything can just be injected anywhere.

Triton is being rewritten as Arbor: https://github.com/matt328/arbor. 

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

- Going to have to abandon OS X for now, seeing as how I'm heavily leaning into Vulkan's DrawIndirect, and the VK_KHR_draw_indirect_count extension isn't available with MoltenVK. I could work around it, but then it's going down the path of having different code paths in the core renderer for different platforms, and also the workaround wouldn't even be all that performant as I'd have to stream the count value back out of the compute shader, and OS X had a whole bunch of other compatibility issues I grew tired of dealing with. So no more OS X.
