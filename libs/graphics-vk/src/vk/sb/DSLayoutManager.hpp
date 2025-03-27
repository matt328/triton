#pragma once

#include "Rando.hpp"

namespace tr {

class DSLayout;
class Device;
class IDebugManager;

using DSLayoutHandle = size_t;

class DSLayoutManager {
public:
  explicit DSLayoutManager(std::shared_ptr<Device> newDevice,
                           std::shared_ptr<IDebugManager> newDebugManager);
  ~DSLayoutManager();

  DSLayoutManager(const DSLayoutManager&) = delete;
  DSLayoutManager(DSLayoutManager&&) = delete;
  auto operator=(const DSLayoutManager&) -> DSLayoutManager& = delete;
  auto operator=(DSLayoutManager&&) -> DSLayoutManager& = delete;

  [[nodiscard]] auto createLayout(vk::DescriptorSetLayoutBinding binding,
                                  std::string_view name) -> DSLayoutHandle;
  [[nodiscard]] auto getLayout(DSLayoutHandle handle) -> DSLayout&;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<IDebugManager> debugManager;

  MapKey keyGen{};
  std::unordered_map<DSLayoutHandle, std::unique_ptr<DSLayout>> layoutMap;
};

}
