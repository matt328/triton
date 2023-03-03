![Build](https://github.com/matt328/triton/actions/workflows/main.yml/badge.svg)

# Triton

Triton is the latest of a 3D game framework that I have been writing off and on with various different technologies
in some form or another for the better part of 20 years now.  The current implementation attempts to use as 
'modern' of C++ as is possible and uses Vulkan for the graphics rendering.  As such I'm using vulkan_raii, which
extends vulkan_hpp (which wraps the Vulkan Vulkan C api with object oriented C++ structures) and allows for the 
use of RAII concepts, removing alot of the difficulty in preventing resource leaks and tearing everything down
in the correct order.

At present there is not much to see here as I'm still working through Vulkan's exceedingly verbose initialization
process.

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

Currently only Windows and Visual Studio 2022 are supported.  I've done the whole cross platform CMake thing, and 
find myself only ever actually coding on Windows, so this is just cleaner.  This project uses vcpkg for dependency
management, so you should be able to just clone the repo, initialize vcpkg and VS will download everything you need
into a `.gitignore`ed folder within the project.

There IS a prereq that needs installed, the Vulkan SDK which you can download from https://vulkan.lunarg.com/

Currently Triton is developed and tested against version `1.3.231.0`.

Download and install the SDK and create an environment variable (bleh I know) named `VULKAN_SDK` that points to e.g.
`C:\VulkanSDK\1.3.231.0`.  It has to point to the folder named with the version number.

1. Clone the repo
2. Init up vcpkg:
```PowerShell
git submodule init
git submodule update
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg.exe integrate install
```
3. Open Triton.sln in VS
4. Click run
5. Be greatly underwhelmed.
