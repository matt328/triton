# TODO

## Project Structure

apps
  cauldron
  assettool
  testarea
libs
  base-kit
    - low level utilities and structs needed by any/all other libraries. put something here only as a last resort
  shared
    - is this even still needed? each component library should abstract its behavior behind a public api, and most of what is currently in shared
      should probably just be part of that libs public api, with things common to more than one component lib in base-kit
    - Standalone utilities that can be used by other libraries, and are generic enough to work with any gameworld or graphics implementation.
  framework
    - Provides a FrameworkContext which keeps gameworld and graphics loosely coupled, and allows client applications to interface with each.
    - houses the run loop and orchestrates how gameworld and graphics interact with each other.
  game-world
    - houses the ecs and is where game logic lives, uses entt, but keeps it abstracted away from other libs
    - another ecs or no ecs at all could be swapped out, but that should be able to happen without changing the public api of the library
  graphics
    - all rendering code goes here
    - vulkan or dx12 or whatever should be able to be swapped out without changing the public api of the library

- Need to clean up and document project configurations.
  - When building without testing, test executables and their dependencies shouldn't even be included
  - Document how to run unit tests and coverage without all the magic vscode jank.

- create some debug rendering utils
  - wireframe cube
  - vertex labels

- Revisit transvoxel algorithm for terrain

- figure out lighting

- character walk around terrain

- skybox/sphere

- Issue Backlog
  - Better Exception Handling
    - create a base exception class that captures the current source_location() and includes it in the what()
    - Decide if an exception should really be thrown all the way out to main() and crash the application.
  - Logging
    - decide if there's any benefit to having something like trace() include the current source_location() in the message.

  - Handle DeviceLost?
    - would have to be able to undo and not do the previous thing that caused device lost.
    - even then only a subset of those things will be recoverable, those may just be things that happen in the editor, or during development. Even Unreal just crashes when the device is lost.
