# Triton

Triton is the latest iteration of a 3D game framework that I have been writing off and on with various different technologies
in some form or another for the better part of 20 years now. The current implementation attempts to use as
'modern' of C++ as is possible and uses Vulkan for the graphics rendering. As such I'm using vulkan_raii, which
extends vulkan_hpp (which wraps the Vulkan Vulkan C api with object oriented C++ structures) and allows for the
use of RAII concepts, removing alot of the difficulty in preventing resource leaks and tearing everything down
in the correct order.

More design details and planning stuff is in the [wiki](https://github.com/matt328/triton/wiki).

## Building/Running

This has ~~recently~~ been migrated to using CMake, and I use vscode primarily for development. So far everything works well on Windows as well as OS X ~~and Linux~~. Linux build is having some issues right now, and I don't have a Linux desktop environment configured at the moment but it'll be back someday.

Dependency management has been migrated into vanilla CMake using FetchContent but the Vulkan SDK still needs installed manually.

1. Install the [Vulkan SDK](https://vulkan.lunarg.com/) and make sure the env var `VULKAN_SDK` gets set appropriately on your system
1. Clone the repo
1. `cmake --preset <preset>`
1. `cmake --build --preset <preset>`

## Testing

Testing is using Catch2, Trompeloeil for mocking, llvm-cov for coverage.
There is a CMake Preset to use to enable coverage, `debug-coverage`.

1. `cmake --preset debug-coverage`
1. `cmake --build --preset --target <test project>`
1. `ninja ccov-export-<test project>`

This will write out an lcov info file into `build/debug-coverage/ccov` and you can configure and IDE plugin to display it in the gutters.

I create a VSCode task to run the ninja target, and use `Coverage Gutters` in watch mode for immediate feedback on how test coverage is progressing and which, if any, branches I might have missed.

## C++ Compiler Notes

CMake Presets exist for clang and Visual Studio 17 2022.

The project *should* explicitly link against libc++ when being compiled with clang. I think 'official' builds should be compiled with clang/libc++, MSVC STL could be used when trying to use address sanitizer, but those should be one-of scenarios used to find a particular bug.

Largely simplified thanks to using Ninja instead of make.  Ninja is cross platform, so no messing around with mingw32 builds, and seems to be a bit faster even with this tiny little project. Just install CMake, LLVM, and Ninja. These should be the same on all 3 platforms now.  The `Visual Studio 17 2022` should generate a valid project, there is a small issue with the assets being located relative to the executable when debugging, but that is currently being worked on. That being said I mostly use ninja, clang, and VSCode for development.

Install [CMake](https://cmake.org/download/).  Make sure `cmake` is on your path.

For Windows, install [LLVM](https://github.com/llvm/llvm-project/releases/)

For Linux you can probably `sudo apt install llvm`.

For MacOS, I install the latest version of LLVM using homebrew.

Make sure `clang` and `clang++` are on your path.

Download [Ninja](https://github.com/ninja-build/ninja/releases). Make sure `ninja` is on your path.

You can set `CC` and `CCX` env vars to `path\to\clang.exe` and `path\to\clang++.exe` if you don't want to risk VSCode detecting the wrong 'kit'. This isn't a big deal, you just have to use the quick menu and select `CMake: Select a Kit` and select the kit that will use clang and clang++ you installed earlier.

The clangd extension gives a much better experience in VSCode than Microsoft's Intellisense. Once you get it configured correctly. Include paths are kind of wonky. See the `.clangd` file at the root of the project.

## OS X Specific Things

- Apple Clang is always inconsistent with llvm, so for some consistency I `brew install llvm` and make sure `CC` and `CXX` env vars point to those compilers' executables
- have to set `MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS=1` in order for the executables to run on OS X.
- TODO: Figure out how to produce an app bundle with CPack so this env var can be bundled in there and you can just double click the app bundle.
- Also TODO: Fix OS X code in general. Some extensions used aren't supported, so factor that stuff out and create an OS X specific variant.
