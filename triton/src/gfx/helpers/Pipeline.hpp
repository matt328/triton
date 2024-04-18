#pragma once

namespace tr::gfx::Helpers {

   using BasicPipelineData =
       std::tuple<std::unique_ptr<vk::raii::Pipeline>, std::unique_ptr<vk::raii::PipelineLayout>>;

   std::string readShaderFile(const std::string_view& filename) {
      if (std::ifstream file(filename.data(), std::ios::binary); file.is_open()) {
         file.seekg(0, std::ios::end);

         const std::streampos fileSize = file.tellg();

         file.seekg(0, std::ios::beg);

         std::string shaderCode;
         shaderCode.resize(fileSize);
         file.read(shaderCode.data(), fileSize);

         file.close();
         return shaderCode;
      }
      std::stringstream ss;
      ss << "Failed to read shader from file " << filename.data();
      throw std::runtime_error(ss.str());
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createBindlessDescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto textureBinding = vk::DescriptorSetLayoutBinding{
          .binding = 3,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 128,
          .stageFlags = vk::ShaderStageFlagBits::eAll};

      const auto bindlessFlags = vk::DescriptorBindingFlagBits::ePartiallyBound |
                                 vk::DescriptorBindingFlagBits::eUpdateAfterBind;

      const auto extendedInfo =
          vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{.bindingCount = 1,
                                                           .pBindingFlags = &bindlessFlags};

      const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{
          .pNext = &extendedInfo,
          .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
          .bindingCount = 1,
          .pBindings = &textureBinding};

      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(dslCreateInfo));
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createPerFrameDescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      const auto createInfo =
          vk::DescriptorSetLayoutCreateInfo{.bindingCount = 1, .pBindings = &binding};
      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(createInfo));
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createSSBODescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto ssboBinding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      const auto createInfo =
          vk::DescriptorSetLayoutCreateInfo{.bindingCount = 1, .pBindings = &ssboBinding};

      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(createInfo));
   }
}