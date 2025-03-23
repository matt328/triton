#pragma once

#include "vk/core/Device.hpp"

namespace tr {

class DSLayout {
public:
  DSLayout(std::shared_ptr<Device> newDevice,
           const std::shared_ptr<IDebugManager>& debugManager,
           const vk::DescriptorSetLayoutCreateInfo& info,
           std::string_view name = "Unnamed Layout");
  ~DSLayout() = default;

  DSLayout(const DSLayout&) = delete;
  auto operator=(const DSLayout&) -> DSLayout& = delete;

  DSLayout(DSLayout&&) = delete;
  auto operator=(DSLayout&&) -> DSLayout& = delete;

  [[nodiscard]] auto getVkLayout() const -> const vk::DescriptorSetLayout*;
  [[nodiscard]] auto getLayoutSize() const -> vk::DeviceSize;
  [[nodiscard]] auto getAlignedSize() const -> vk::DeviceSize;
  [[nodiscard]] auto getBindingOffset(uint32_t binding) const -> vk::DeviceSize;

private:
  std::shared_ptr<Device> device;

  std::unique_ptr<vk::raii::DescriptorSetLayout> vkLayout;
};

}
