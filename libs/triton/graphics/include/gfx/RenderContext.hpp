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
      explicit RenderContext(GLFWwindow* window,
                             bool guiEnabled = false,
                             bool validationEnabled = false);
      ~RenderContext();

      RenderContext(const RenderContext&) = delete;
      auto operator=(const RenderContext&) -> RenderContext& = delete;

      RenderContext(RenderContext&&) = delete;
      auto operator=(RenderContext&&) -> RenderContext& = delete;

      void render() const;
      void waitIdle() const;

      void setCurrentCameraData(cm::gpu::CameraData& cameraData) const;

      void setResizeListener(const std::function<void(std::pair<uint32_t, uint32_t>)>& fn) const;

      void setRenderData(const cm::gpu::RenderData& renderData) const;

      void setDebugRendering(bool wireframeEnabled) const;

      // Resource Creation
      [[nodiscard]] auto createStaticModel(const std::filesystem::path& modelPath) const
          -> cm::ModelData;

      [[nodiscard]] auto createTerrain() const -> std::vector<cm::ModelData>;

      [[nodiscard]] auto createSkinnedModel(const std::filesystem::path& modelPath) const
          -> cm::ModelData;

      [[nodiscard]] auto createAABBGeometry(const glm::vec3& min,
                                            const glm::vec3& max) -> cm::ModelData;

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };
}