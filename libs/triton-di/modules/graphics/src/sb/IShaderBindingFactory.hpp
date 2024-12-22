#pragma once

#include <vulkan/vulkan.hpp>
namespace tr {

   class ShaderBinding;

   enum class ShaderBindingHandle : uint32_t {
      Invalid = 0,
      PerFrame = 1,
      Bindless = 2,
      ObjectData = 3,
      AnimationData = 4,
      // TODO(Matt): Add an additional one here for DebugGroup
   };

   class IShaderBindingFactory {
    public:
      IShaderBindingFactory() = default;
      virtual ~IShaderBindingFactory() = default;

      IShaderBindingFactory(const IShaderBindingFactory&) = default;
      IShaderBindingFactory(IShaderBindingFactory&&) = delete;
      auto operator=(const IShaderBindingFactory&) -> IShaderBindingFactory& = default;
      auto operator=(IShaderBindingFactory&&) -> IShaderBindingFactory& = delete;

      [[nodiscard]] virtual auto createShaderBinding(ShaderBindingHandle handle) const
          -> std::unique_ptr<ShaderBinding> = 0;
   };

}
