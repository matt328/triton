#pragma once

#include "IShaderBindingFactory.hpp"
#include "bk/HandleGenerator.hpp"
#include "vk/sb/DSLayoutManager.hpp"

namespace tr {

class Device;
class DSLayoutManager;
class IDebugManager;
class IFrameManager;

class DSShaderBindingFactory : public IShaderBindingFactory {
public:
  explicit DSShaderBindingFactory(std::shared_ptr<Device> newDevice,
                                  std::shared_ptr<DSLayoutManager> newLayoutManager,
                                  std::shared_ptr<IFrameManager> newFrameManager,
                                  std::shared_ptr<IDebugManager> newDebugManager);
  ~DSShaderBindingFactory();

  DSShaderBindingFactory(const DSShaderBindingFactory&) = delete;
  auto operator=(const DSShaderBindingFactory&) -> DSShaderBindingFactory& = delete;
  DSShaderBindingFactory(DSShaderBindingFactory&&) = delete;
  auto operator=(DSShaderBindingFactory&&) -> DSShaderBindingFactory& = delete;

  /// Allocates a new ShaderBinding
  [[nodiscard]] auto createShaderBinding(ShaderBindingType type, Handle<DSLayout> layoutHandle)
      -> LogicalHandle<IShaderBinding> override;

  [[nodiscard]] auto getShaderBinding(Handle<IShaderBinding> handle) -> IShaderBinding& override;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<DSLayoutManager> layoutManager;
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<IDebugManager> debugManager;

  HandleGenerator<IShaderBinding> handleGenerator;
  std::unique_ptr<vk::raii::DescriptorPool> permanentPool;

  std::unordered_map<Handle<IShaderBinding>, std::unique_ptr<IShaderBinding>> shaderBindingMap;
};

}
