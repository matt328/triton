#pragma once

#include "sb/ShaderBinding.hpp"

namespace tr::gfx::pipe {

   const std::filesystem::path SHADERS = std::filesystem::current_path() / "assets" / "shaders";

   class IPipeline {
    public:
      IPipeline() = default;
      virtual ~IPipeline() = default;

      IPipeline(const IPipeline&) = default;
      IPipeline(IPipeline&&) = delete;
      auto operator=(const IPipeline&) -> IPipeline& = default;
      auto operator=(IPipeline&&) -> IPipeline& = delete;

      virtual void bind(const vk::raii::CommandBuffer& cmd) = 0;
      virtual void applyShaderBinding(const sb::ShaderBinding& binding) = 0;
   };

}
