#pragma once

namespace tr::gfx::mem {
   class Buffer;
}

namespace tr::gfx::sb {
   class ShaderBinding {
    public:
      ShaderBinding() = default;
      ShaderBinding(const ShaderBinding&) = default;
      ShaderBinding& operator=(const ShaderBinding&) = default;

      ShaderBinding(ShaderBinding&&) = default;
      ShaderBinding& operator=(ShaderBinding&&) = default;
      virtual ~ShaderBinding() = default;

      virtual void bindBuffer(const int binding, const mem::Buffer& buffer, const size_t size) = 0;
      virtual void update() = 0;
   };
}