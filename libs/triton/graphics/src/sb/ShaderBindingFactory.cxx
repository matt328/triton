#include "sb/ShaderBindingFactory.hpp"
#include "DSShaderBinding.hpp"

#include "GraphicsDevice.hpp"
#include "ShaderBinding.hpp"
#include "LayoutFactory.hpp"
#include "Layout.hpp"

namespace tr::gfx::sb {
   ShaderBindingFactory::ShaderBindingFactory(const GraphicsDevice& graphicsDevice,
                                              const ds::LayoutFactory& layoutFactory,
                                              const bool useDescriptorBuffers)
       : graphicsDevice{graphicsDevice},
         layoutFactory{layoutFactory},
         useDescriptorBuffers{useDescriptorBuffers} {
      if (!useDescriptorBuffers) {
         const auto poolSize = std::array{
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
             graphicsDevice.getVulkanDevice().createDescriptorPool(poolInfo, nullptr));
      }
   }

   ShaderBindingFactory::~ShaderBindingFactory() {
   }

   auto ShaderBindingFactory::createShaderBinding(ShaderBindingHandle handle)
       -> std::unique_ptr<ShaderBinding> {

      if (handle == ShaderBindingHandle::PerFrame) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice.getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eUniformBuffer,
             layoutFactory.getLayout(ds::LayoutHandle::PerFrame).getVkLayout(),
             "Per Frame DS");
      }

      if (handle == ShaderBindingHandle::ObjectData) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice.getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eStorageBuffer,
             layoutFactory.getLayout(ds::LayoutHandle::ObjectData).getVkLayout(),
             "Object Data DS");
      }

      if (handle == ShaderBindingHandle::Bindless) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice.getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eCombinedImageSampler,
             layoutFactory.getLayout(ds::LayoutHandle::Bindless).getVkLayout(),
             "Bindless DS");
      }

      if (handle == ShaderBindingHandle::AnimationData) {
         return std::make_unique<DSShaderBinding>(
             graphicsDevice.getVulkanDevice(),
             **permanentPool,
             vk::DescriptorType::eStorageBuffer,
             layoutFactory.getLayout(ds::LayoutHandle::AnimationData).getVkLayout(),
             "Animation Data DS");
      }

      Log::error << "unhandled, uh, handle when creating shaderbinding: " << std::endl;
      assert(true);

      return nullptr;
   }
}
