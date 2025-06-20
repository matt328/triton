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
    -> Handle<DSLayout> {
  const auto handle = handleGenerator.requestHandle();

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

  layoutMap[handle] = std::make_unique<DSLayout>(device, debugManager, dslCreateInfo, name);

  return handle;
}

auto DSLayoutManager::getLayout(Handle<DSLayout> handle) -> DSLayout& {
  assert(layoutMap.contains(handle));
  return *layoutMap.at(handle);
}

}
