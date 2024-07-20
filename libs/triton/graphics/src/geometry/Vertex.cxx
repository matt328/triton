#include "as/Vertex.hpp"

#include "Vertex.hpp"

namespace tr::gfx::geo {
   vk::VertexInputBindingDescription VertexBuilder::vertexInputBindingDescription;
   std::vector<vk::VertexInputAttributeDescription> VertexBuilder::vertexInputAttributeDescriptions;
   vk::PipelineVertexInputStateCreateInfo VertexBuilder::pipelineVertexInputStateCreateInfo;

   vk::VertexInputBindingDescription VertexBuilder::inputBindingDescription(
       const uint32_t binding) {
      return {.binding = binding,
              .stride = sizeof(as::Vertex),
              .inputRate = vk::VertexInputRate::eVertex};
   }

   vk::VertexInputAttributeDescription VertexBuilder::inputAttributeDescription(
       const uint32_t binding,
       const uint32_t location,
       const VertexComponent component) {
      switch (component) {
         case VertexComponent::Position:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat};
         case VertexComponent::Normal:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(as::Vertex, normal)};
         case VertexComponent::UV:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32Sfloat,
                    .offset = offsetof(as::Vertex, uv)};
         case VertexComponent::Color:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(as::Vertex, color)};
         case VertexComponent::Tangent:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(as::Vertex, tangent)};
         case VertexComponent::Joint0:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR8G8B8A8Uint,
                    .offset = offsetof(as::Vertex, joint0)};
         case VertexComponent::Weight0:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32A32Sfloat,
                    .offset = offsetof(as::Vertex, weight0)};
         default:
            return {};
      }
   }

   std::vector<vk::VertexInputAttributeDescription> VertexBuilder::inputAttributeDescriptions(
       const uint32_t binding,
       const std::span<VertexComponent> components) {
      std::vector<vk::VertexInputAttributeDescription> result;
      uint32_t location = 0;
      for (const auto component : components) {
         result.push_back(VertexBuilder::inputAttributeDescription(binding, location, component));
         location++;
      }
      return result;
   }

   /** @brief Returns the default pipeline vertex input state create info structure for the
    * requested vertex components */
   vk::PipelineVertexInputStateCreateInfo* VertexBuilder::getPipelineVertexInputState(
       const std::span<VertexComponent> components) {
      vertexInputBindingDescription = VertexBuilder::inputBindingDescription(0);
      VertexBuilder::vertexInputAttributeDescriptions =
          VertexBuilder::inputAttributeDescriptions(0, components);
      pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
      pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions =
          &VertexBuilder::vertexInputBindingDescription;
      pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
          static_cast<uint32_t>(VertexBuilder::vertexInputAttributeDescriptions.size());
      pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions =
          VertexBuilder::vertexInputAttributeDescriptions.data();
      return &pipelineVertexInputStateCreateInfo;
   }
}