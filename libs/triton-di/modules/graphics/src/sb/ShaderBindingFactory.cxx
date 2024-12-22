#include "ShaderBindingFactory.hpp"

#include "gfx/IGraphicsDevice.hpp"

#include "DSShaderBinding.hpp"
#include "ShaderBinding.hpp"
#include "LayoutFactory.hpp"
#include "Layout.hpp"

namespace tr {
   ShaderBindingFactory::ShaderBindingFactory(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                                              std::shared_ptr<LayoutFactory> newLayoutFactory,
                                              RenderContextConfig rendererConfig)
       : config{rendererConfig},
         graphicsDevice{std::move(newGraphicsDevice)},
         layoutFactory{std::move(newLayoutFactory)} {

      Log.debug("Constructing ShaderBindingFactory, useDescriptorBuffers: {0}",
                rendererConfig.useDescriptorBuffers);

      if (!rendererConfig.useDescriptorBuffers) {
         constexpr auto poolSize = std::array{
             vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                    .descriptorCount = 3 * 10},
             vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                    .descriptorCount = 3 * 100},
             vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage,
                                    .descriptorCount = 3 * 10},
             vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer,
                                    .descriptorCount = 3 * 10},
         };

         const vk::DescriptorPoolCreateInfo poolInfo{
             .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                      vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT,
             .maxSets = 3 * 10 * static_cast<uint32_t>(poolSize.size()),
             .poolSizeCount = poolSize.size(),
             .pPoolSizes = poolSize.data()};

         permanentPool = std::make_unique<vk::raii::DescriptorPool>(
             graphicsDevice->getVulkanDevice()->createDescriptorPool(poolInfo, nullptr));
      }
   }

   ShaderBindingFactory::~ShaderBindingFactory() { // NOLINT(*-use-equals-default)
   }

   auto ShaderBindingFactory::createShaderBinding(const ShaderBindingHandle handle) const
       -> std::unique_ptr<ShaderBinding> {

      if (handle == ShaderBindingHandle::PerFrame) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice->getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eUniformBuffer,
             layoutFactory->getLayout(LayoutHandle::PerFrame).getVkLayout(),
             "Per Frame DS");
      }

      if (handle == ShaderBindingHandle::ObjectData) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice->getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eStorageBuffer,
             layoutFactory->getLayout(LayoutHandle::ObjectData).getVkLayout(),
             "Object Data DS");
      }

      if (handle == ShaderBindingHandle::Bindless) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice->getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eCombinedImageSampler,
             layoutFactory->getLayout(LayoutHandle::Bindless).getVkLayout(),
             "Bindless DS");
      }

      if (handle == ShaderBindingHandle::AnimationData) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice->getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eStorageBuffer,
             layoutFactory->getLayout(LayoutHandle::AnimationData).getVkLayout(),
             "Animation Data DS");
      }

      Log.error("Unhandled, um, handle I guess when creating ShaderBinding");
      assert(true);

      return nullptr;
   }
}
