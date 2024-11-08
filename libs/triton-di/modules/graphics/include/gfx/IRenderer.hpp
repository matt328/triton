#pragma once

namespace tr::gfx {
   class IRenderer {
    public:
      explicit IRenderer() = default;
      virtual ~IRenderer() = default;

      IRenderer(const IRenderer&) = default;
      IRenderer(IRenderer&&) = delete;
      auto operator=(const IRenderer&) -> IRenderer& = default;
      auto operator=(IRenderer&&) -> IRenderer& = delete;

      virtual void render() = 0;
      virtual void waitIdle() = 0;
   };
}