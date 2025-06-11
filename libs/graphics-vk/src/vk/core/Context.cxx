#include "Context.hpp"

namespace tr {
Context::Context() {
  context = std::make_unique<vk::raii::Context>();
}

Context::~Context() {
  Log.trace("Destroying context");
}

auto Context::getVkContext() const -> const vk::raii::Context& {
  return *context;
}

auto Context::getExtensionProperties() const -> std::vector<vk::ExtensionProperties> {
  return context->enumerateInstanceExtensionProperties();
}

auto Context::getLayerProperties() const -> std::vector<vk::LayerProperties> {
  return context->enumerateInstanceLayerProperties();
}

auto Context::createInstance(const vk::InstanceCreateInfo& createInfo) const
    -> std::unique_ptr<vk::raii::Instance> {
  return std::make_unique<vk::raii::Instance>(*context, createInfo);
}

auto Context::getVulkanVersion() const -> std::string {
  auto version = context->enumerateInstanceVersion();
  const auto versionString = std::format("{}.{}.{}",
                                         VK_VERSION_MAJOR(version),
                                         VK_VERSION_MINOR(version),
                                         VK_VERSION_PATCH(version));
  return versionString;
}
}
