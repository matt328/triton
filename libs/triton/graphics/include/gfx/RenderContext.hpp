#pragma once

#include "cm/Handles.hpp"
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

      void setCurrentCameraData(cm::gpu::CameraData&& cameraData);

      void setResizeListener(const std::function<void(std::pair<uint32_t, uint32_t>)>& fn);

      void setRenderData(cm::gpu::RenderData& renderData);

      void setDebugRendering(bool wireframeEnabled);

      // Resource Creation
      auto createStaticModel(const std::filesystem::path& modelPath)
          -> futures::cfuture<cm::ModelData>;

      auto createTerrain(const uint32_t size) -> futures::cfuture<cm::ModelData>;

      auto createSkinnedModel(const std::filesystem::path& modelPath)
          -> futures::cfuture<cm::ModelData>;

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };
}