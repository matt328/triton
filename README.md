# Triton

Triton is the latest iteration of a 3D game framework that I have been writing off and on with various different technologies in some form or another for quite some time. The current implementation attempts to use as 'modern' of C++ as is possible and uses Vulkan for the graphics rendering. It is using vulkan_raii, which extends vulkan_hpp (which wraps the Vulkan Vulkan C api with object oriented C++ structures) and allows for the use of RAII concepts, which makes preventing resource leaks and ensuring resources are deallocated in the proper order less error prone.

More design details and planning stuff is in the [wiki](https://github.com/matt328/triton/wiki).

## Building/Running

The project is using CMake, and I use VSCode primarily for development. So far everything works well on Windows as well as OS X and Linux.

Dependency management uses vanilla CMake using FetchContent but for now the Vulkan SDK still needs installed manually.

### Build Steps

1. Install the [Vulkan SDK](https://vulkan.lunarg.com/) and make sure the env var `VULKAN_SDK` gets set appropriately on your system
1. Install [CMake](https://cmake.org/download/).  Make sure `cmake` is on your path.
1. For Windows, install [LLVM](https://github.com/llvm/llvm-project/releases/)
1. For Linux you can probably `sudo apt install llvm`.
1. For MacOS, I install the latest version of LLVM using homebrew.
1. Make sure `clang` and `clang++` are on your path.
1. Download [Ninja](https://github.com/ninja-build/ninja/releases). Make sure `ninja` is on your path.
1. Clone the repo
1. `cmake --preset debug`
1. `cmake --build --preset debug`

## Testing

Testing is using Catch2, Trompeloeil for mocking, llvm-cov for coverage.
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
- debug-coverage - debug build but with coverage enabled
- release - optimized release build

I prefer the clangd extension in VSCode. Include paths are kind of wonky. See the `.clangd` file at the root of the project.

## OS X Notes

- For some consistency I `brew install llvm` and make sure `CC` and `CXX` env vars point to those compilers' executables
- have to set `MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS=1` in order for the executables to run on OS X.
- TODO: Figure out how to produce an app bundle with CPack so this env var can be bundled in there and you can just double click the app bundle.
- At present, everything should work on OS X.
