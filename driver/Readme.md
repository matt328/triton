# Render Layer

- takes in a list of things to render every frame
- `queueRenderObjects(std::vector<RenderObject>);`

# Scene Layer

- Produces a list of things that need rendered every frame
- culling, octree, etc
- `renderObject(renderable);`
- this should happen on the GPU via compute shaders, so it may be combined with the Render Layer

# Game Layer

- implemented by the Game class below

# ApplicationContext

- Renderer
- ResourceManager
- Application
- Game
- actionSet
- mediates communication between then

## Renderer

- is the class `Context` for now.

## ResourceManager

- Handles loading and referencing of meshes and textures
- When loading a Resource, the manager loads the resource from filename or bytes and places it's gpu
  resource references inside the Context and returns a handle

## Application

- Has game loop timing implementation
- Emits key/mouse/gamepad events to be exposed by the ApplicationContext

## Game

- Owns Entity objects
  - entity system is a hybrid of GameObject heirarchy and ECS.
  - Still have a generic top level GameObject, but each object has a list of components attached
    to it.
- Exposes methods for crudding Entities
  - GameClient will create entities via API, either directly or from loading a 'level'
  - An impl of IRenderable component can have a reference to the Renderer
  - Entities can be updated via an IController component, might be hard coded behavior, or a ScriptedController
  - Can be deleted via API

## actionSet

- Emits Actions based on listening to events from application, and custom sources

# General

- for simplicity, remove all the pimpls, just move game logic into drive

# GameClient

## LevelController

- Contains a data structure representing the level's data file
- reading in the level data file and then setting up the entity system should set the game into motion.
