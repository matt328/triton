#include "DSLayoutManager.hpp"
#include "DSLayout.hpp"

namespace tr {

DSLayoutManager::DSLayoutManager(std::shared_ptr<Device> newDevice,
                                 std::shared_ptr<IDebugManager> newDebugManager)
    : device{std::move(newDevice)}, debugManager{std::move(newDebugManager)} {
}

DSLayoutManager::~DSLayoutManager() {
}

auto DSLayoutManager::createLayout(vk::DescriptorSetLayoutBinding binding, std::string_view name)
    -> DSLayoutHandle {
  const auto key = keyGen.getKey();

  static constexpr vk::DescriptorBindingFlags bindlessFlags =
      vk::DescriptorBindingFlagBits::ePartiallyBound;

  constexpr auto extendedInfo =
      vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{.bindingCount = 1,
                                                       .pBindingFlags = &bindlessFlags};

  const auto useDescriptorBuffers = false;
  vk::DescriptorSetLayoutCreateFlags flags{};
  if (useDescriptorBuffers) {
    flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
  }

  const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{.pNext = &extendedInfo,
                                                               .flags = flags,
                                                               .bindingCount = 1,
                                                               .pBindings = &binding};

  layoutMap[key] = std::make_unique<DSLayout>(device, debugManager, dslCreateInfo, name);

  return key;
}

auto DSLayoutManager::getLayout(DSLayoutHandle handle) -> DSLayout& {
  assert(layoutMap.contains(handle));
  return *layoutMap.at(handle);
}

}
