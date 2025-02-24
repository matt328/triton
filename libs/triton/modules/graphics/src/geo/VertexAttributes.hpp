#pragma once

namespace tr {
enum class VertexComponent {
  Position,
  Normal,
  UV,
  Color,
  Tangent,
  Joint0,
  Weight0
};

struct VertexBuilder {
  static vk::VertexInputBindingDescription vertexInputBindingDescription;
  static std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
  static vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;

  static vk::VertexInputBindingDescription inputBindingDescription(uint32_t binding);

  static vk::VertexInputAttributeDescription inputAttributeDescription(uint32_t binding,
                                                                       uint32_t location,
                                                                       VertexComponent component);

  static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions(
      uint32_t binding,
      std::span<VertexComponent> components);

  /** @brief Returns the default pipeline vertex input state create info structure for the
   * requested vertex components */
  static vk::PipelineVertexInputStateCreateInfo* getPipelineVertexInputState(
      std::span<VertexComponent> components);
};
}
