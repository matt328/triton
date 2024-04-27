#include "Vertex.hpp"

namespace tr::gfx::geo {
   vk::VertexInputBindingDescription Vertex::vertexInputBindingDescription;
   std::vector<vk::VertexInputAttributeDescription> Vertex::vertexInputAttributeDescriptions;
   vk::PipelineVertexInputStateCreateInfo Vertex::pipelineVertexInputStateCreateInfo;

   vk::VertexInputBindingDescription Vertex::inputBindingDescription(const uint32_t binding) {
      return {.binding = binding,
              .stride = sizeof(Vertex),
              .inputRate = vk::VertexInputRate::eVertex};
   }

   vk::VertexInputAttributeDescription Vertex::inputAttributeDescription(
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
                    .offset = offsetof(Vertex, normal)};
         case VertexComponent::UV:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, uv)};
         case VertexComponent::Color:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, color)};
         case VertexComponent::Tangent:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, tangent)};
         case VertexComponent::Joint0:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, joint0)};
         case VertexComponent::Weight0:
            return {.location = location,
                    .binding = binding,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, weight0)};
         default:
            return {};
      }
   }

   std::vector<vk::VertexInputAttributeDescription> Vertex::inputAttributeDescriptions(
       const uint32_t binding,
       const std::vector<VertexComponent> components) {
      std::vector<vk::VertexInputAttributeDescription> result;
      uint32_t location = 0;
      for (const auto component : components) {
         result.push_back(Vertex::inputAttributeDescription(binding, location, component));
         location++;
      }
      return result;
   }

   /** @brief Returns the default pipeline vertex input state create info structure for the
    * requested vertex components */
   vk::PipelineVertexInputStateCreateInfo* Vertex::getPipelineVertexInputState(
       const std::vector<VertexComponent> components) {
      vertexInputBindingDescription = Vertex::inputBindingDescription(0);
      Vertex::vertexInputAttributeDescriptions = Vertex::inputAttributeDescriptions(0, components);
      pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
      pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions =
          &Vertex::vertexInputBindingDescription;
      pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
          static_cast<uint32_t>(Vertex::vertexInputAttributeDescriptions.size());
      pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions =
          Vertex::vertexInputAttributeDescriptions.data();
      return &pipelineVertexInputStateCreateInfo;
   }
}