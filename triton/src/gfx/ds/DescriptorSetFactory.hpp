#pragma once

namespace tr::gfx::ds {

   class DescriptorSet;
   class LayoutFactory;

   enum class SetHandle : uint32_t {
      Invalid = 0,
      PerFrame = 1,
      Bindless = 2,
      ObjectData = 3,
      AnimationData = 3,
   };

   class DescriptorSetFactory {
    public:
      DescriptorSetFactory(const vk::raii::Device& device,
                           const LayoutFactory& layoutFactory,
                           const uint32_t framesInFlightCount);
      ~DescriptorSetFactory();

      DescriptorSetFactory(const DescriptorSetFactory&) = delete;
      DescriptorSetFactory& operator=(const DescriptorSetFactory&) = delete;

      DescriptorSetFactory(DescriptorSetFactory&&) = delete;
      DescriptorSetFactory& operator=(DescriptorSetFactory&&) = delete;

      [[nodiscard]] DescriptorSet& getDescriptorSet(SetHandle handle,
                                                    const uint32_t frameIndex) const;

    private:
      void initPermanentPool(const vk::raii::Device& device, const uint32_t framesInFlightCount);

      void initBindlessDescriptorSet(const vk::raii::Device& device,
                                     const LayoutFactory& layoutFactory,
                                     const size_t descriptorCount);

      void initPerFrameDescriptorSet(const vk::raii::Device& device,
                                     const LayoutFactory& layoutFactory,
                                     const size_t descriptorCount);

      void initObjectDataDescriptorSet(const vk::raii::Device& device,
                                       const LayoutFactory& layoutFactory,
                                       const size_t descriptorCount);

      void initAnimationDescriptorSet(const vk::raii::Device& device,
                                      const LayoutFactory& layoutFactory,
                                      const size_t descriptorCount);

      std::unique_ptr<vk::raii::DescriptorPool> permanentPool;

      std::unordered_map<SetHandle, std::vector<std::unique_ptr<DescriptorSet>>> descriptorSetCache;
   };
}