#pragma once
#include "Context.hpp"
#include "gfx/IWindow.hpp"
#include "gfx/IDebugManager.hpp"

namespace tr {

class Instance {
public:
  explicit Instance(std::shared_ptr<Context> newContext,
                    std::shared_ptr<IDebugManager> newDebugManager,
                    std::shared_ptr<IWindow> newWindow);
  ~Instance();

  Instance(const Instance&) = delete;
  Instance(Instance&&) = delete;
  auto operator=(const Instance&) -> Instance& = delete;
  auto operator=(Instance&&) -> Instance& = delete;

  auto createSurface(IWindow& window) const -> std::unique_ptr<vk::raii::SurfaceKHR>;

  auto getPhysicalDevices() const -> std::vector<vk::raii::PhysicalDevice>;

  /// @brief Escape hatch for ImGui's Vulkan API. Figure out how to close this at some point.
  [[nodiscard]] auto getVkInstance() -> vk::Instance;

private:
  std::shared_ptr<Context> context;
  std::shared_ptr<IDebugManager> debugManager;
  std::shared_ptr<IWindow> window;

  std::unique_ptr<vk::raii::Instance> instance;

  [[nodiscard]] auto getInstanceExtensions() const -> std::vector<const char*>;
  static auto isPortabilityRequired(std::span<const char*> extensionNames) -> bool;
};

}
