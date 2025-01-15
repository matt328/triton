#pragma once

#include "vk/sb/DSLayoutManager.hpp"
namespace tr {

class IShaderBinding;

using ShaderBindingHandle = size_t;

enum class ShaderBindingType : uint32_t {
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
                                                 DSLayoutHandle layoutHandle)
      -> ShaderBindingHandle = 0;

  [[nodiscard]] virtual auto getShaderBinding(ShaderBindingHandle handle) -> IShaderBinding& = 0;
};

}
