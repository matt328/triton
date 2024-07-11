#pragma once

#include "cm/ObjectData.hpp"
#include "cm/RenderData.hpp"

#include <entt/signal/fwd.hpp>

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

      [[nodiscard]] auto& getResourceManager() const;

      void render();
      void waitIdle();

      void enqueueRenderObject(RenderObject renderObject);

      void setCurrentCameraData(cm::CameraData&& cameraData);

      void setResizeListener(const std::function<void(std::pair<uint32_t, uint32_t>)>& fn);

      void setRenderData(cm::RenderData& renderData);

      void setDebugRendering(bool wireframeEnabled);

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };
}