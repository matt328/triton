#include "DescriptorSetFactory.hpp"
#include "DescriptorSet.hpp"
#include "LayoutFactory.hpp"

namespace tr::gfx::ds {
   DescriptorSetFactory::DescriptorSetFactory(const vk::raii::Device& device,
                                              const LayoutFactory& layoutFactory,
                                              const uint32_t framesInFlightCount) {
      initPermanentPool(device, framesInFlightCount);
      initBindlessDescriptorSet(device, layoutFactory, static_cast<size_t>(framesInFlightCount));
      initPerFrameDescriptorSet(device, layoutFactory, static_cast<size_t>(framesInFlightCount));
      initObjectDataDescriptorSet(device, layoutFactory, static_cast<size_t>(framesInFlightCount));
      initAnimationDescriptorSet(device, layoutFactory, static_cast<size_t>(framesInFlightCount));
   }

   DescriptorSetFactory::~DescriptorSetFactory() {
   }

   DescriptorSet& DescriptorSetFactory::getDescriptorSet(SetHandle handle,
                                                         const uint32_t frameIndex) const {
      return *descriptorSetCache.at(handle)[frameIndex];
   }

   void DescriptorSetFactory::initPermanentPool(const vk::raii::Device& device,
                                                const uint32_t framesInFlightCount) {
      const auto poolSize = std::array{
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                 .descriptorCount = framesInFlightCount * 10},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                 .descriptorCount = framesInFlightCount * 100},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage,
                                 .descriptorCount = framesInFlightCount * 10},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer,
                                 .descriptorCount = framesInFlightCount * 10},
      };

      const vk::DescriptorPoolCreateInfo poolInfo{
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                   vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT,
          .maxSets = framesInFlightCount * 10 * static_cast<uint32_t>(poolSize.size()),
          .poolSizeCount = poolSize.size(),
          .pPoolSizes = poolSize.data()};

      permanentPool = std::make_unique<vk::raii::DescriptorPool>(
          device.createDescriptorPool(poolInfo, nullptr));
   }

   void DescriptorSetFactory::initBindlessDescriptorSet(const vk::raii::Device& device,
                                                        const LayoutFactory& layoutFactory,
                                                        const size_t descriptorCount) {
      auto sets = std::vector<std::unique_ptr<DescriptorSet>>{};
      for (size_t i = 0; i < descriptorCount; i++) {
         sets.emplace_back(
             std::make_unique<DescriptorSet>(device,
                                             layoutFactory.getVkLayout(LayoutHandle::Bindless),
                                             **permanentPool,
                                             vk::DescriptorType::eCombinedImageSampler));
      }
      descriptorSetCache.emplace(SetHandle::Bindless, std::move(sets));
   }

   void DescriptorSetFactory::initPerFrameDescriptorSet(const vk::raii::Device& device,
                                                        const LayoutFactory& layoutFactory,
                                                        const size_t descriptorCount) {
      auto sets = std::vector<std::unique_ptr<DescriptorSet>>{};
      for (size_t i = 0; i < descriptorCount; i++) {
         sets.emplace_back(
             std::make_unique<DescriptorSet>(device,
                                             layoutFactory.getVkLayout(LayoutHandle::PerFrame),
                                             **permanentPool,
                                             vk::DescriptorType::eUniformBuffer));
      }
      descriptorSetCache.emplace(SetHandle::PerFrame, std::move(sets));
   }

   void DescriptorSetFactory::initObjectDataDescriptorSet(const vk::raii::Device& device,
                                                          const LayoutFactory& layoutFactory,
                                                          const size_t descriptorCount) {
      auto sets = std::vector<std::unique_ptr<DescriptorSet>>{};
      for (size_t i = 0; i < descriptorCount; i++) {
         sets.emplace_back(
             std::make_unique<DescriptorSet>(device,
                                             layoutFactory.getVkLayout(LayoutHandle::ObjectData),
                                             **permanentPool,
                                             vk::DescriptorType::eStorageBuffer));
      }
      descriptorSetCache.emplace(SetHandle::ObjectData, std::move(sets));
   }

   void DescriptorSetFactory::initAnimationDescriptorSet(const vk::raii::Device& device,
                                                         const LayoutFactory& layoutFactory,
                                                         const size_t descriptorCount) {
      auto sets = std::vector<std::unique_ptr<DescriptorSet>>{};
      for (size_t i = 0; i < descriptorCount; i++) {
         sets.emplace_back(
             std::make_unique<DescriptorSet>(device,
                                             layoutFactory.getVkLayout(LayoutHandle::AnimationData),
                                             **permanentPool,
                                             vk::DescriptorType::eStorageBuffer));
      }
      descriptorSetCache.emplace(SetHandle::AnimationData, std::move(sets));
   }
}