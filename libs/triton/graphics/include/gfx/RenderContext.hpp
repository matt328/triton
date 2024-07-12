#pragma once

#include "cm/ObjectData.hpp"
#include "cm/RenderData.hpp"

namespace tr::gfx {
   struct RenderObject;
   struct PerFrameData;
   class ImmediateContext;
   class AbstractPipeline;
   class Frame;
   class RendererBase;
   class Pipeline;
   class PipelineBuilder;

   namespace tx {
      class ResourceManager;
   }

   namespace ds {
      class LayoutFactory;
      class DescriptorSetFactory;
   }

   namespace Gui {
      class ImGuiHelper;
   }

   namespace mem {
      class Allocator;
      class Image;
   }

   using ResizeDelegateType = entt::delegate<void(std::pair<uint32_t, uint32_t>)>;

   class RenderContext {
    public:
      RenderContext(GLFWwindow* window, bool guiEnabled = false);
      ~RenderContext();

      RenderContext(const RenderContext&) = delete;
      RenderContext& operator=(const RenderContext&) = delete;

      RenderContext(RenderContext&&) = delete;
      RenderContext& operator=(RenderContext&&) = delete;

      void render();
      void waitIdle();

      void enqueueRenderObject(RenderObject renderObject);

      void setCurrentCameraData(cm::CameraData&& cameraData);

      void setResizeListener(const std::function<void(std::pair<uint32_t, uint32_t>)>& fn);

      void setRenderData(cm::RenderData& renderData);

      void setDebugRendering(bool wireframeEnabled);

      // Resource Creation
      auto createTerrain() -> std::future<cm::ModelHandle>;
      auto loadModelAsync(const std::filesystem::path& modelPath) -> std::future<cm::ModelHandle>;
      auto loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                 const std::filesystem::path& skeletonPath,
                                 const std::filesystem::path& animationPath)
          -> std::future<cm::LoadedSkinnedModelData>;

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };
}