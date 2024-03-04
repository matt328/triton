# Design Notes

## Architecture Overview

### Triton (Engine Itself, static lib, `tr`)

#### Context `tr::ctx`

- Contains GameplaySystem and Renderer
- Handles decoupling the two so that renderer can be the only place where the graphics api is directly used.

#### Gameplay `tr::ctx::gp`

- Contain the ECS
- Provides GameplayAPI

#### Renderer `tr::ctx::gfx`

- Handles rendering and resource creation/management
- is completely unaware of which platform it's running on, and which windowing system has been used. The applications should be responsible for abstracting that away

### Editor (executable `tr::ed`

- Contains Imgui that can be handed to the renderer to be drawn
- Wires up window events, input callbacks etc into the context
- Wires up events Imgui needs
- Bridges Imgui components' inputs into the GameplayAPI
- Knows how to query the GamplayAPI in order to store game file(s)
- Knows how to read in game file(s) and set up the game using the GamplayAPI

### Game (executable `tr::gm`)

- Wires up things like window events, input callbacks, etc into the context.
- Reads optimized game files.
- Knows how to call the GameplayAPI with game file(s) information to set up and start the game.

## Component Detail

### Context

- News up a GameplaySystem and a Renderer, mediates communication between the two so they can stay decoupled.
- Recieves some sort of abstracted native window handle to be passed into the renderer's ctor so it knows where to render its output. (Vulkan will cast this to a GLFWwindow* in order to create the vk::Surface).
- Provides clients with a way to register application level callbacks and route them to the systems that need them.
- Provides clients access to the GameplayAPI.

### Gameplay API

Implemented as GameplayFacade class.

This API will sit on top of the ECS and essentially be an abstraction layer over it (like I didn't want to do).  This API will be available to the scripts attached to entities.  It will also be directly usable via the Editor's components as well.

API Needs:

- set state on a given entity (including adding/removing script components)
- query state of other entities
- create entities
- destroy entities
- create/read/update context data

Are these all anything will need to make a game? There may be certain things that make sense to build into the engine, terrain generation/paging, day night cycle for example.

### Editor

Editor should produce the imgui draw data and context should provide a listener to recieve changes in the imgui draw data each frame.

Context and renderer will only contain Imgui functions to render the draw data, nothing in here produces any draw data.

Editor Application will be the only component that calls any imgui functions to actually render any imgui components.

Imgui components in Editor's Application can have direct access to the GameplayFacade, this should be all they need to access in order to do what they need to do.

Imgui's edit components work by taking in a reference to some data to edit. I see 2 options here:

1. Create a copy of all the data in the Application and push that data into the GameplayFacade each frame
2. Have the GameplayFacade just expose references to all the data the gui will edit.

All data the editor will edit *should* be contained within the components of the ECS, so #2 is probably cleaner.

Should the script interface be the GameplayFacade as well? Probably, I think I decided that earlier.  Just need to figure out converting glm to and from Lua. I don't know, maybe we shouldn't be doing vector operations in a scripting language, and favor doing them in C++ where they're fast.

TODO:

1. Editor Load/Save Game Project
   1. Update gltf loading to account for referenced image files
   1. Load in new entities - pick a gltf file.
   1. Save state of the 'game', basically entities and their transforms
   1. load the state of the 'game'
1. Scripting for entity behaviors
   - Attach a script to an entity
   - Edit scripts inside the UI
   - Expose the script interface to a UI
1. Renderer Improvements
   - Lighting
   - Draw Indiret
   - GPU Culling
1. Finish Out Gamepad Support
1. Terrain Generating/Rendering

### Editor Save Data

Initially it seems like the whole game file could be a big gltf file, but the data will be more than just geometry and materials, there will be scripts attached to entities, cameras, environmental things. Effects and actions that are more than just geometry and materials.

EnTT's serialization is a little too bare bones and experimental feeling right now, also we don't want to just snapshot the entire state of the game.  For now start a json format that can be easily read, written and parsed, that should be good enough since it's only for the editor.  An actual game's format will need to be more optimized.

File Format see `.\project.json`

Right now, just a list of entities and their components.

Maybe have a EditorInfo component that only the editor uses to keep track of the assets used to create a given Renderable.  Could make the existing NameComponent more general until there is a need for the editor to query on separate components.
