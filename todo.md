# TODO

## Threading Issues

There are some multithreading issues around resource loading.
Appears on loading either first batch or second batch.
Observations gathered here.

- Doesn't seem to happen when debugging, only running
- An image that shouldn't be, is having barriers recorded for transitions as though it's a texture that's just been loaded, but the image is the depth image, or a swapchain image. It seems to only be one image at a time.
- A texture will have a barrier transferring ownership from transfer queue to graphics queue queued multiple times.
- It works fine.

## Project Structure

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
