#pragma once

namespace tr::gfx::geo {
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
      glm::u8vec4 joint0;
      glm::vec4 weight0;
      glm::vec4 tangent;

      bool operator==(const Vertex& other) const {
         return pos == other.pos && normal == other.normal && uv == other.uv &&
                color == other.color && joint0 == other.joint0 && weight0 == other.weight0 &&
                tangent == other.tangent;
      }

      template <class Archive>
      void serialize(Archive& archive) {
         archive(pos, normal, uv, color, joint0, weight0, tangent);
      }

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
          const std::span<VertexComponent> components);

      /** @brief Returns the default pipeline vertex input state create info structure for the
       * requested vertex components */
      static vk::PipelineVertexInputStateCreateInfo* getPipelineVertexInputState(
          const std::span<VertexComponent> components);
   };
}