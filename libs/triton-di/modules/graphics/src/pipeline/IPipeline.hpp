#pragma once

#include "sb/ShaderBinding.hpp"

namespace tr::gfx::pipe {
   class IPipeline {
    public:
      IPipeline() = default;
      virtual ~IPipeline() = default;

      virtual void bind(const vk::raii::CommandBuffer& cmd) = 0;
      virtual void applyShaderBinding(const sb::ShaderBinding& binding) = 0;
   };

}
