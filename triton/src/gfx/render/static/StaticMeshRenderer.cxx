#include "StaticMeshRenderer.hpp"

#include "gfx/Pipeline.hpp"
#include "gfx/GraphicsDevice.hpp"
#include "gfx/DescriptorSetLayouts.hpp"

#include "gfx/dss/DescriptorAllocator.hpp"
#include "gfx/geometry/Vertex.hpp"
#include "gfx/helpers/Vulkan.hpp"
#include "gfx/dss/DescriptorSetBuilder.hpp"

#include "util/Paths.hpp"

namespace tr::gfx::render {
   StaticMeshRenderer::StaticMeshRenderer(const GraphicsDevice& graphicsDevice,
                                          dss::DescriptorSetLayoutCache* layoutCache)
       : graphicsDevice{graphicsDevice}, layoutCache{layoutCache} {
      initPipeline(graphicsDevice, layoutCache);
   }

   StaticMeshRenderer::~StaticMeshRenderer() {
   }

   void StaticMeshRenderer::resize(const vk::Extent2D newSize) {
      pipeline->resize(newSize);
   }

   void StaticMeshRenderer::draw(const vk::raii::CommandBuffer& cmd) {
   }

   void StaticMeshRenderer::initPipeline(const GraphicsDevice& graphicsDevice,
                                         dss::DescriptorSetLayoutCache* layoutCache) {

      const auto perFrameDSL =
          layoutCache->createDescriptorSetLayout(&PerFrameDescriptorSet::LayoutCreateInfo);

      const auto objectDataDSL =
          layoutCache->createDescriptorSetLayout(&ObjectDataDescriptorSet::LayoutCreateInfo);

      const auto textureDSL =
          layoutCache->createDescriptorSetLayout(&TextureDescriptorSet::LayoutCreateInfo);

      const auto setLayouts = std::array{**textureDSL, **objectDataDSL, **perFrameDSL};

      vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{.setLayoutCount = setLayouts.size(),
                                                            .pSetLayouts = setLayouts.data()};

      // Configure Vertex Attributes
      const auto bindingDescription = Geometry::Vertex::inputBindingDescription(0);
      const auto attributeDescriptions =
          Geometry::Vertex::inputAttributeDescriptions(0,
                                                       {Geometry::VertexComponent::Position,
                                                        Geometry::VertexComponent::Color,
                                                        Geometry::VertexComponent::UV});

      const auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
          .vertexBindingDescriptionCount = 1,
          .pVertexBindingDescriptions = &bindingDescription,
          .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
          .pVertexAttributeDescriptions = attributeDescriptions.data()};

      const auto colorFormat = vk::Format::eR16G16B16A16Sfloat;
      const auto depthFormat = Helpers::findDepthFormat(graphicsDevice.getPhysicalDevice());

      const auto renderingCreateInfo =
          vk::PipelineRenderingCreateInfo{.colorAttachmentCount = 1,
                                          .pColorAttachmentFormats = &colorFormat,
                                          .depthAttachmentFormat = depthFormat};

      pipeline = std::make_unique<Pipeline>(graphicsDevice,
                                            pipelineLayoutCreateInfo,
                                            vertexInputStateCreateInfo,
                                            renderingCreateInfo,
                                            util::Paths::SHADERS / "shader.vert",
                                            util::Paths::SHADERS / "shader.frag");
   }
}
