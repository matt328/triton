#pragma once

namespace Triton::Graphics {
   enum class VertexComponent {
      Position,
      Normal,
      UV,
      Color,
      Tangent,
      Joint0,
      Weight0
   };

   struct Vertex {
      glm::vec3 pos;
      glm::vec3 normal;
      glm::vec2 uv;
      glm::vec4 color;
      glm::vec4 joint0;
      glm::vec4 weight0;
      glm::vec4 tangent;
      static vk::VertexInputBindingDescription vertexInputBindingDescription;
      static std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
      static vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;

      static vk::VertexInputBindingDescription inputBindingDescription(uint32_t binding);

      static vk::VertexInputAttributeDescription inputAttributeDescription(
          uint32_t binding,
          uint32_t location,
          VertexComponent component);

      static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions(
          uint32_t binding,
          const std::vector<VertexComponent> components);

      /** @brief Returns the default pipeline vertex input state create info structure for the
       * requested vertex components */
      static vk::PipelineVertexInputStateCreateInfo* getPipelineVertexInputState(
          const std::vector<VertexComponent> components);
   };
}