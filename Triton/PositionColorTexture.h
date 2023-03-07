#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace VertexFormats {
   struct PositionColorTexture {
      glm::vec3 pos;
      glm::vec3 color;
      glm::vec2 texCoord;

      static vk::VertexInputBindingDescription bindingDescription() {
         constexpr auto stride = static_cast<uint32_t>(sizeof(PositionColorTexture));
         constexpr vk::VertexInputBindingDescription description{
             .binding = 0, .stride = stride, .inputRate = vk::VertexInputRate::eVertex};

         return description;
      }

      static std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions() {
         const std::array attributeDescriptions{
             vk::VertexInputAttributeDescription{.location = 0,
                                                 .binding = 0,
                                                 .format = vk::Format::eR32G32B32Sfloat,
                                                 .offset = offsetof(PositionColorTexture, pos)},
             vk::VertexInputAttributeDescription{.location = 1,
                                                 .binding = 0,
                                                 .format = vk::Format::eR32G32B32Sfloat,
                                                 .offset = offsetof(PositionColorTexture, color)},
             vk::VertexInputAttributeDescription{.location = 2,
                                                 .binding = 0,
                                                 .format = vk::Format::eR32G32Sfloat,
                                                 .offset =
                                                     offsetof(PositionColorTexture, texCoord)}};

         return attributeDescriptions;
      }

      bool operator==(const PositionColorTexture& other) const {
         return pos == other.pos && color == other.color && texCoord == other.texCoord;
      }
   };
}