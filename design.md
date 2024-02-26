# Design Notes

## Modules

1. triton
   - renderer
   - gameplay module (ecs)
   - Context that manages these two.
1. editor
   - Application
   - Editor Component Renderers
   - Editor Adapter that can be installed into the renderer via the frontend, and allows the editor components to be rendered by the renderer backend
   - Editor Core that handles communication between editor components and gameplay module
1. game
   - Application
   - basically just defines gameplay logic and calls gameplay.runGame()
   - also has callbacks from the context for handling saving gamestate, windowing things, etc.

This is feeling overengineered already, so to start:

1. create a Context class that handles creating all the resources needed
   - initially it will just own a Game and a Renderer.
   - Come up with a better name for 'Game' it's basically the application of EnTT.
2. The application will just call `context.run()`.
3. Eventually context.run() will take in a thing.  That thing will be some kind of Application, either a GameApplication or an EditorApplication.
4. There probably will always be a Game/Editor pair with a shared Gameplay module between them.  The Game will just blindly execute the Gameplay module, while the editor will mainly execute the Gameplay module, but will also be able to micromanage everything about the Gameplay at any time.
5. The Gameplay module might be a shared library that can be recompiled and reloaded at runtime by the editor.  Replicating that kind of functionality with a scripting system would be cool, but not nearly as powerful.
6. Think about if context.run() could just take in a gameplay module.  I think the editor would need to be a completely standalone application that might call something like context.run(gameplayModule) and then call context.stop() then context.run(gameplayModule) after the shared library has been rebuilt and reloaded.
