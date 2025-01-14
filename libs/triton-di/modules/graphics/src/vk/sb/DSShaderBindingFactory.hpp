#pragma once

#include "IShaderBindingFactory.hpp"
#include "cm/Rando.hpp"
#include "vk/sb/DSLayoutManager.hpp"

namespace tr {

class Device;
class DSLayoutManager;
class IDebugManager;

class DSShaderBindingFactory : public IShaderBindingFactory {
public:
  explicit DSShaderBindingFactory(std::shared_ptr<Device> newDevice,
                                  std::shared_ptr<DSLayoutManager> newLayoutManager,
                                  std::shared_ptr<IDebugManager> newDebugManager);
  ~DSShaderBindingFactory() override = default;

  DSShaderBindingFactory(const DSShaderBindingFactory&) = delete;
  auto operator=(const DSShaderBindingFactory&) -> DSShaderBindingFactory& = delete;

  DSShaderBindingFactory(DSShaderBindingFactory&&) = delete;
  auto operator=(DSShaderBindingFactory&&) -> DSShaderBindingFactory& = delete;

  /// Allocates a new ShaderBinding
  [[nodiscard]] auto createShaderBinding(ShaderBindingType type, DSLayoutHandle layoutHandle)
      -> ShaderBindingHandle override;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<DSLayoutManager> layoutManager;
  std::shared_ptr<IDebugManager> debugManager;

  MapKey keyGen;
  std::unordered_map<ShaderBindingHandle, std::unique_ptr<IShaderBinding>> shaderBindingMap;
  std::unique_ptr<vk::raii::DescriptorPool> permanentPool;
};

}
