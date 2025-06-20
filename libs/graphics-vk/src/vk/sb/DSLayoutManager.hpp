#pragma once

#include "bk/HandleGenerator.hpp"
#include "bk/Rando.hpp"

namespace tr {

class DSLayout;
class Device;
class IDebugManager;

class DSLayoutManager {
public:
  explicit DSLayoutManager(std::shared_ptr<Device> newDevice,
                           std::shared_ptr<IDebugManager> newDebugManager);
  ~DSLayoutManager();

  DSLayoutManager(const DSLayoutManager&) = delete;
  DSLayoutManager(DSLayoutManager&&) = delete;
  auto operator=(const DSLayoutManager&) -> DSLayoutManager& = delete;
  auto operator=(DSLayoutManager&&) -> DSLayoutManager& = delete;

  [[nodiscard]] auto createLayout(vk::DescriptorSetLayoutBinding binding, std::string_view name)
      -> Handle<DSLayout>;
  [[nodiscard]] auto getLayout(Handle<DSLayout> handle) -> DSLayout&;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<IDebugManager> debugManager;

  HandleGenerator<DSLayout> handleGenerator;
  std::unordered_map<Handle<DSLayout>, std::unique_ptr<DSLayout>> layoutMap;
};

}
