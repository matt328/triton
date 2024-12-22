#include "VertexAttributes.hpp"
#include "as/Vertex.hpp"

namespace tr {
   vk::VertexInputBindingDescription VertexBuilder::vertexInputBindingDescription;
   std::vector<vk::VertexInputAttributeDescription> VertexBuilder::vertexInputAttributeDescriptions;
   vk::PipelineVertexInputStateCreateInfo VertexBuilder::pipelineVertexInputStateCreateInfo;

   auto VertexBuilder::inputBindingDescription(const uint32_t binding)
       -> vk::VertexInputBindingDescription {
      return {.binding = binding,
              .stride = sizeof(as::Vertex),
              .inputRate = vk::VertexInputRate::eVertex};
   }

   auto VertexBuilder::inputAttributeDescription(const uint32_t binding,
                                                 const uint32_t location,
                                                 const VertexComponent component)
       -> vk::VertexInputAttributeDescription {
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
                    .format = vk::Format::eR32G32B32A32Sfloat,
                    .offset = offsetof(as::Vertex, color)};
         case VertexComponent::Tangent:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32A32Sfloat,
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

   auto VertexBuilder::inputAttributeDescriptions(const uint32_t binding,
                                                  const std::span<VertexComponent> components)
       -> std::vector<vk::VertexInputAttributeDescription> {
      std::vector<vk::VertexInputAttributeDescription> result;
      uint32_t location = 0;
      for (const auto component : components) {
         result.push_back(inputAttributeDescription(binding, location, component));
         location++;
      }
      return result;
   }

   /** @brief Returns the default pipeline vertex input state create info structure for the
    * requested vertex components */
   auto VertexBuilder::getPipelineVertexInputState(const std::span<VertexComponent> components)
       -> vk::PipelineVertexInputStateCreateInfo* {
      vertexInputBindingDescription = inputBindingDescription(0);
      vertexInputAttributeDescriptions = inputAttributeDescriptions(0, components);
      pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
      pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions =
          &VertexBuilder::vertexInputBindingDescription;
      pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
          static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
      pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions =
          vertexInputAttributeDescriptions.data();
      return &pipelineVertexInputStateCreateInfo;
   }
}