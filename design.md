# Design Notes

## Debug Renderer

Each debug rendered item is going to be an entity in the ECS. A vertex buffer will need to be updated as the entities
are created and destroyed.

Try out just encapsulating a buffer in an object that can have a smart pointer passed around instead of a handle. This
object would be like a GeometryGroup that can encapsulate the creation and updating of a vertex buffer. It's api can
handle adding new geometries, and removing them, internally tracking their elements in the buffer and indexing them by
entity id.

Look into having these geometry groups be able to render themselves, initially via a single bind and multiple draw
calls.

Multithreading concerns:
The issue is we need a consistent copy of the data to be rendered and copying renderdata handles ensures that i think?

Use these as an example of multiple types of things that need drawn so we can improve this process a bit.

An entity can have multiple meshes, and also multiple instances of each mesh.
Meshes instances can belong to multiple entities.

GeometryGroup should also internally maintain an ObjectData buffer that contains a minimum a model matrix for the time
being. This can help ensure that vertex/index buffers, transform data, instance data and the object data buffer all stay
coherent.

1 instance of a mesh.
Only a vertex buffer, not indexed yet.

addMeshInstance() - add vertex data and create an instance, returning both meshId and instanceId
removeInstance() - removes the instance and vertex data

render()

- Get rid of this loop with drawIndirect
- first just build the buffer on the CPU
- eventually send this GeometryGroup data to a compute shader and have the compute shader fill in the buffer based on
  frustum culling, etc
  for ({id, meshData} : meshDataMap) {
  first instance will always be 0, since there will be one ObjectData buffer per GeometryGroup
  when drawing instanced, one draw call will cause the vertex shader to be called `instanceCount` times,
  incrementing the gl_InstanceIndex variable each time

  const auto vertexCount = meshData.vertexCount;
  const auto instanceCount = meshData.instanceData.size();
  const auto firstVertex = meshData.vertexOffset;
  const auto firstInstance = 0;

  cmd.draw(vertexCount, instanceCount, firstVertex, firstIndex);
  }

GeometryGroup:

- `addMeshInstance(entityId, vertexData) -> {meshId, instanceId}`
- `removeInstance(entityId, instanceId) -> void`

- `render(vk::raii::CommandBuffer&)`

- `std::unique_ptr<mem::Buffer> vertexBuffer`
- `std::unordered_map<EntityId, MeshData> meshDataMap`

MeshData

- `uint32_t vertexCount`
- `int32_t vertexOffset`
- `std::vector<InstanceData> instanceData`

InstanceData

- `uint32_t instanceId`
- `uint32_t instanceOffset`

- init()
    - shaders
    - ShaderBindings
    - pipelines
    - Dynamic Vertex/Index Buffers
    - Transform buffer
    - Color buffer
    - common uniform buffer

- update()
    - copy buffers

- render()
    - bind buffers
    - draw indexed from the store

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
- is completely unaware of which platform it's running on, and which windowing system has been used. The applications
  should be responsible for abstracting that away

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
- Recieves some sort of abstracted native window handle to be passed into the renderer's ctor so it knows where to
  render its output. (Vulkan will cast this to a GLFWwindow* in order to create the vk::Surface).
- Provides clients with a way to register application level callbacks and route them to the systems that need them.
- Provides clients access to the GameplayAPI.

### Gameplay API

Implemented as GameplayFacade class.

This API will sit on top of the ECS and essentially be an abstraction layer over it (like I didn't want to do). This API
will be available to the scripts attached to entities. It will also be directly usable via the Editor's components as
well.

API Needs:

- set state on a given entity (including adding/removing script components)
- query state of other entities
- create entities
- destroy entities
- create/read/update context data

Are these all anything will need to make a game? There may be certain things that make sense to build into the engine,
terrain generation/paging, day night cycle for example.

### Editor

Editor should produce the imgui draw data and context should provide a listener to recieve changes in the imgui draw
data each frame.

Context and renderer will only contain Imgui functions to render the draw data, nothing in here produces any draw data.

Editor Application will be the only component that calls any imgui functions to actually render any imgui components.

Imgui components in Editor's Application can have direct access to the GameplayFacade, this should be all they need to
access in order to do what they need to do.

Imgui's edit components work by taking in a reference to some data to edit. I see 2 options here:

1. Create a copy of all the data in the Application and push that data into the GameplayFacade each frame
2. Have the GameplayFacade just expose references to all the data the gui will edit.

All data the editor will edit *should* be contained within the components of the ECS, so #2 is probably cleaner.

Should the script interface be the GameplayFacade as well? Probably, I think I decided that earlier. Just need to figure
out converting glm to and from Lua. I don't know, maybe we shouldn't be doing vector operations in a scripting language,
and favor doing them in C++ where they're fast.

### Resource Updates

#### Models

- Offline process models from fbx into a model, animation, and skeleton
    - use the offline libs from ozz-animation to do this all in one step.

- Skeletons are what binds an animation to a model. Can have multiple animations per skeleton, and a skeleton can work
  for multiple models.
- The editor should have a frontend for the model tool, browse to a model file, and it runs the conversion and outputs
  the results into a working dir.
- For now, develop an executable tool that takes in an fbx file and produces a model, skeleton, and animation.
- The editor will just load in all 3 of these at once until the animation system is actually working
- The model file should just contain vertices and indices, and a map of vertex joint indices to skeleton joint indices
  so that the skeleton joints can be reordered into what the vertex data has.
- Milestone 1 is to be able to load a triplet of model, skeleton, animation, and have the debug UI able to test out the
  animation.
- Milestone 2 is to be able to associate multiple animations with a single model
- Milestone 3 is to be able to blend between animations eg: idle->walking->running-sprinting.

- rework the way assets in general are handled, the cmake targets are starting to feel clunky
- can be handled differently with the editor application as opposed to an exported game.
- editor should just access and load them from an external private 'assets' repository
- exported game should have them packaged up in some optimized format.
- Fonts: look at how Walnut embeds the font data and do that with the editor
- Models/Textures - see how gltf loading could automatically find and load textures referenced by gltf files.
- Shaders - The game currently loads and compiles text files into shaders, this should be moved into the editor level,
  an exported game won't be doing that, and in that case, the editor can just treat them like any other asset that is
  needed to initialize itself, like the icon.

1. Embed the font inside the code
1. Make static mesh loading just take a gltf file name and that's it
1. Move the shader compilation into the editor, and have the engine just take compiled spirv bytecode blobs.

### Misc Stuff to research

- ImGui logger output
- ImGui entity json viewer/editor
- ImGui Tests
- Terrain Generation/Rendering
- DrawIndirect
- Lighting
- Scripting

### Editor Save Data

Initially it seems like the whole game file could be a big gltf file, but the data will be more than just geometry and
materials, there will be scripts attached to entities, cameras, environmental things. Effects and actions that are more
than just geometry and materials.

EnTT's serialization is a little too bare bones and experimental feeling right now, also we don't want to just snapshot
the entire state of the game. For now start a json format that can be easily read, written and parsed, that should be
good enough since it's only for the editor. An actual game's format will need to be more optimized.

File Format see `.\project.json`

Right now, just a list of entities and their components.

Maybe have a EditorInfo component that only the editor uses to keep track of the assets used to create a given
Renderable. Could make the existing NameComponent more general until there is a need for the editor to query on separate
components. Edit: this worked out well.

#### TODO

- Think about what an 'unloaded' state looks like. Should it be blank with just Imgui rendered, or should it start with
  a hardcoded scene like blender.
- Think about 'camera' more. Maybe the editor should always just supply a camera and don't bother writing it to the
  file, only reset its position when you load a different project.
    - Probably don't do too much with this until there's a need for multiple cameras, as it will change then anyway.
- Handle errors instead of crashing.

## Game World/Renderer Interaction

```mermaid
---
title: Game World Classes
---
classDiagram
    namespace ctx {
        class Context {
            +std:: unique_ptr~GameplayFacade~ gameplayFacade
            +std:: unique_ptr~GameplaySystem~ gameplaySystem
            +std:: unique_ptr~RenderContext~ renderContext
            start() void
        }

        class GameplayFacade {
            +GameplaySystem& gameplaySystem
            +RenderContext& renderer
            create*Entity()  EntityType
            setCurrentCamera() void
        }
    }

    namespace gp {
        class GameplaySystem {
            +std:: unique_ptr~EntitySystem~ entitySystem
            +std:: unique_ptr~ActionSystem~ actionSystem
            +std:: unique_ptr~AnimationFactory~ animationFactory
            create*Entity()  EntityType
            fixedUpdate() void
            update() void
        }

        class BehaviorSystem {
        }

        class ActionSystem {
        }

        class AnimationFactory {
        }
    }
    namespace gfx {
        class RenderContext {
            +std:: unique_ptr~FrameManager~ frameManager
            +std:: unique_ptr~RenderGroup~ debugGroup
            +std:: unique_ptr~ResourceManager~ resourceManager
        }

        class ResourceManager {
            +std:: unique_ptr~GeometryFactory~ geometryFactory
            +std:: vector~ImmutableMesh~ meshList
            +std:: vector~Textures::Texture~ textureList
            getMesh( MeshHandle) ImmutableMesh&
            getTextures()  LockableResource~std::vector~vk::DescriptorImageInfo~~
            createModel(const std:: filesystem:: path&)  ModelData
            createStaticMesh(const geo:: GeometryData& geometry)  MeshHandle
        }

        class FrameManager {
        }

        class GeometryFactory {
        }

        class RenderGroup {
        }
    }
    Context --* GameplayFacade
    Context --* GameplaySystem
    Context --* RenderContext
    GameplayFacade --> GameplaySystem
    GameplayFacade --> RenderContext
    GameplaySystem --* EntitySystem
    GameplaySystem --* ActionSystem
    GameplaySystem --* AnimationFactory
    RenderContext --* FrameManager
    RenderContext --* RenderGroup
    RenderContext --* ResourceManager
    ResourceManager --* GeometryFactory
```

```mermaid
sequenceDiagram
    box rgba(0, 255, 0, 0.2) GameWorld
        participant Editor
        participant GameplayFacade
        participant Context
        participant GameplaySystem
        participant EntitySystem
    end
    box rgba(0, 0, 255, 0.2) Renderer
        participant RenderContext
        participant ResourceManager
        participant GeometryFactory
    end

    rect rgba(0, 0, 0, 0.3)
        Editor ->> GameplayFacade: createStaticModel()
        activate GameplayFacade
        GameplayFacade ->> RenderContext: createResource()
        RenderContext ->> ResourceManager: createResource()
        ResourceManager ->> GeometryFactory: createGeometry()
        GeometryFactory -->> ResourceManager: geometryHandle
        ResourceManager -->> RenderContext: resourceHandle
        RenderContext -->> GameplayFacade: resourceHandle
        GameplayFacade ->> GameplaySystem: createEntity(resourceHandle)
        GameplaySystem ->> EntitySystem: createEntity(resourceHandle)
        EntitySystem -->> GameplaySystem: entityId
        GameplaySystem -->> GameplayFacade: entityId
        GameplayFacade -->> Editor: entityId
        deactivate GameplayFacade
    end

    rect rgba(0, 0, 0, 0.3)
        loop Main
            loop Physics Updates
                Context ->> GameplaySystem: fixedUpdate()
                activate Context
                GameplaySystem ->> EntitySystem: fixedUpdate()
                activate EntitySystem
                EntitySystem ->> EntitySystem: Update All Entities
                EntitySystem -->> GameplaySystem: fixedUpdateComplete
                deactivate EntitySystem
                GameplaySystem -->> Context: fixedUpdateComplete
            end

            Context ->> GameplaySystem: update()
            GameplaySystem ->> EntitySystem: update()
            activate EntitySystem
            EntitySystem ->> EntitySystem: Extract RenderData
            EntitySystem -->> GameplaySystem: renderData
            deactivate EntitySystem
            GameplaySystem ->> RenderContext: setRenderData()
            RenderContext ->> ResourceManager: setRenderData()
            ResourceManager -->> RenderContext: renderDataRecieved
            RenderContext -->> GameplaySystem: renderDataRecieved
            Context ->> RenderContext: render()
            deactivate Context
        end
    end

```

```mermaid
---
title: RenderGroup
---
classDiagram
    class RenderGroup {
        addMesh(std:: vector~as::Vertex~) size_t
        addInstance(size_t meshId) void
        render() void
        +std:: vector~MeshData~ meshList
        +std:: shared_ptr~MultiBuffer~
    }

    class MeshData {
        +uint32_t vertexOffset
        +uint32_t vertexCount
        +uint32_t firstInstance
        +uint32_t instanceCount
    }

    class MultiBuffer {
        +std:: unique_ptr~mem::Buffer~~ buffer
    }

    RenderGroup --* MeshData
    RenderGroup --* MultiBuffer

```

RenderGroup::render()

The components in the ECS will hold the game world data for the transform and visibility of each instance.

```mermaid
classDiagram
    class RenderGroupComponent {
        +size_t meshId
        +size_t instanceId
        +glm:: mat4 transform
        +bool visible
    }
```

Preparing render data should put instance data into a

```c
std::unordered_map<meshId, std::unordered_map<instanceId, InstanceData>>
```

so that the RenderGroup can construct a single storage buffer containing the InstanceData, corresponding to the order of
the meshes and instances. I don't think the order of the instances matters, so long as each bucket of instance data
matches the correct mesh.

# Render Job System

```mermaid
graph TD
    subgraph Frame Management
        FrameManager["FrameManager"]
    end

    subgraph Render Jobs
        RenderJob1["RenderJob1"]
        RenderJob2["RenderJob2"]
        RenderScheduler["RenderScheduler"]
    end

    subgraph Vulkan Pipeline
        Pipeline["Pipeline"]
        DescriptorManager["DescriptorManager"]
        RenderPassManager["RenderPassManager"]
    end

    subgraph Resource Management
        CommandBufferManager["CommandBufferManager"]
        ResourceManager["ResourceManager"]
        SynchronizationManager["SynchronizationManager"]
    end

    DebugTools["Debug & Validation Tools"]
%% Workflow connections
    FrameManager --> RenderScheduler
    RenderScheduler --> RenderJob1
    RenderScheduler --> RenderJob2
    RenderJob1 -->|Uses| Pipeline
    RenderJob1 -->|Uses| DescriptorManager
    RenderJob2 -->|Uses| Pipeline
    RenderJob2 -->|Uses| DescriptorManager
    Pipeline --> RenderPassManager
    DescriptorManager --> RenderPassManager
    RenderScheduler --> CommandBufferManager
    CommandBufferManager -->|Submits| SynchronizationManager
    CommandBufferManager --> ResourceManager
    RenderPassManager --> CommandBufferManager
    SynchronizationManager --> FrameManager
    DebugTools -.-> RenderScheduler
    DebugTools -.-> ResourceManager
    DebugTools -.-> CommandBufferManager

```

```mermaid
classDiagram
%% Core Classes
    class FrameManager {
        +reset()
        +getCurrentFrameResources()
    }

    class RenderScheduler {
        +enqueueJob(RenderJob)
        +executeJobs(VkCommandBuffer)
    }

    class RenderJob {
        <<abstract>>
        +execute(VkCommandBuffer)
    }

    class Pipeline {
        +bind(VkCommandBuffer)
        -VkPipeline vkPipeline
        -VkPipelineLayout vkPipelineLayout
    }

    class DescriptorManager {
        +allocateDescriptorSets(VkDescriptorSetLayout)
        +bindDescriptorSets(VkCommandBuffer, VkPipelineLayout, descriptorSets)
        -VkDescriptorPool descriptorPool
    }

    class CommandBufferManager {
        +beginRecording() VkCommandBuffer
        +endRecording(VkCommandBuffer)
        +submit(VkCommandBuffer, VkQueue)
        -VkCommandPool commandPool
    }

    class ResourceManager {
        +allocateBuffer(size, usage, properties) VkBuffer
        +allocateImage(...)
        +releaseResource(resource)
        -VkDeviceMemory memory
    }

    class RenderPassManager {
        +createRenderPass(attachments, subpasses)
        +getFramebuffer(renderPass)
        -VkRenderPass renderPass
        -VkFramebuffer framebuffer
    }

    class SynchronizationManager {
        +waitForFence(VkFence)
        +createSemaphore() VkSemaphore
        +createFence() VkFence
        -VkSemaphore semaphore
        -VkFence fence
    }

%% Relationships
    RenderScheduler --> RenderJob: executes
    RenderJob --> Pipeline: uses
    RenderJob --> DescriptorManager: uses
    Pipeline --> RenderPassManager: uses
    RenderScheduler --> CommandBufferManager: submits commands
    CommandBufferManager --> ResourceManager: uses
    RenderPassManager --> ResourceManager: allocates attachments
    CommandBufferManager --> SynchronizationManager: synchronizes
    SynchronizationManager --> FrameManager: resets frame resources
%% Optional Component
    class DebugTools {
        +enableValidationLayers()
        +logPerformance()
        +debugObject(name, handle)
    }
    DebugTools --> RenderScheduler: observes
    DebugTools --> ResourceManager: observes
    DebugTools --> CommandBufferManager: observes

```

1. RenderContext

The RenderContext acts as the core manager for rendering resources, state, and the overall rendering pipeline. It serves
as a central hub that ties everything together.

Responsibilities:

    Initialization & Configuration: Create Vulkan objects like the device, swapchain, and basic synchronization primitives (semaphores, fences, etc.).
    Global Resource Management:
        Manage persistent resources such as shaders, pipelines, and descriptors.
        Provide access to shared components like the CommandBufferManager, ResourceManager, or SyncManager.
    Lifecycle Management:
        Handle initialization and cleanup of the Vulkan instance and resources.
        Provide entry points for starting and stopping the rendering process.
    Interface:
        Expose methods to begin a frame, end a frame, and submit rendering work.
        Act as a mediator between the FrameManager, RenderScheduler, and other components.

2. FrameManager

The FrameManager is responsible for orchestrating the rendering of individual frames. It ensures synchronization and
prepares the per-frame resources.

Responsibilities:

    Frame Acquisition:
        Acquire the next swapchain image from the RenderContext.
        Manage synchronization primitives (fences and semaphores) for ensuring GPU-CPU synchronization.
    Frame Resource Allocation:
        Allocate per-frame resources (e.g., command buffers, transient buffers) and pass them to the Frame.
        Manage in-flight frames to ensure the correct reuse of resources.
    Submission Management:
        Collect and submit command buffers to the Vulkan queue.
        Chain submissions with semaphores and fences to enforce dependencies.

3. Frame

The Frame represents a snapshot of rendering-specific resources and state for a single frame. It acts as a container for
all per-frame data.

Responsibilities:

    Per-Frame State:
        Hold command buffers, synchronization objects, and other resources (e.g., framebuffers, render passes) for the current frame.
        Store references to transient data used for rendering tasks, such as per-frame uniform buffers.
    Ownership of Command Buffers:
        Store command buffers (acquired from a CommandBufferManager) for recording rendering commands.
        Provide methods or data structures for render tasks to record their commands.
    Execution Hooks:
        Offer methods like begin() and end() to encapsulate rendering logic (e.g., starting and ending render passes).
    Synchronization State:
        Track synchronization objects (fences and semaphores) associated with the frame.

4. RenderScheduler

The RenderScheduler is responsible for managing the execution of render tasks for a frame. It schedules and organizes
the pipeline stages and their dependencies.

Responsibilities:

    Render Task Management:
        Register and configure render tasks during initialization.
        Manage dependencies between tasks (e.g., ensure the depth pre-pass runs before the main shading pass).
    Command Recording:
        Use the Frame to record commands for each render task.
        Set up pipeline barriers or transitions between tasks to ensure resource availability.
    Execution Scheduling:
        Define the order of render task execution within a frame.
        Handle parallel recording of command buffers if supported.
    Task-Specific Resources:
        Prepare and bind render-specific resources (e.g., descriptors, pipelines) required for each task.

Workflow for Rendering a Frame

Here’s how these components interact during the rendering of a complete frame:

    Start of Frame:
        RenderContext initializes the rendering process for the current frame.
        FrameManager acquires a Frame and retrieves the next swapchain image, along with synchronization primitives (e.g., image-available semaphore).

    Command Buffer Preparation:
        The FrameManager resets and allocates command buffers for the frame.
        The RenderScheduler iterates through registered render tasks and requests the Frame's command buffers to record commands.
        The RenderScheduler ensures proper barriers are in place and coordinates task execution.

    Task Execution:
        Each render task records its commands into command buffers using the resources provided by the Frame.
        Barriers and transitions (managed by the RenderScheduler) ensure resources (e.g., images, buffers) are in the correct state for each task.

    Submission:
        Once all tasks are recorded, the FrameManager submits the command buffers to the Vulkan queue using the appropriate semaphores and fences.
        RenderContext handles the final presentation of the swapchain image.

    End of Frame:
        FrameManager updates in-flight frame tracking and recycles resources (e.g., fences, command buffers).
        The RenderContext prepares for the next frame.
