#pragma once

#include "bk/Handle.hpp"
#include "vk/sb/DSLayoutManager.hpp"
namespace tr {

class IShaderBinding;

enum class ShaderBindingType : uint8_t {
  Invalid = 0,
  Textures = 1,
};

class IShaderBindingFactory {
public:
  IShaderBindingFactory() = default;
  virtual ~IShaderBindingFactory() = default;

  IShaderBindingFactory(const IShaderBindingFactory&) = default;
  IShaderBindingFactory(IShaderBindingFactory&&) = delete;
  auto operator=(const IShaderBindingFactory&) -> IShaderBindingFactory& = default;
  auto operator=(IShaderBindingFactory&&) -> IShaderBindingFactory& = delete;

  [[nodiscard]] virtual auto createShaderBinding(ShaderBindingType type,
                                                 Handle<DSLayout> layoutHandle)
      -> LogicalHandle<IShaderBinding> = 0;

  [[nodiscard]] virtual auto getShaderBinding(Handle<IShaderBinding> handle) -> IShaderBinding& = 0;
};

}
