#pragma once

namespace tr::gfx::rd {
   class IRenderer {
    public:
      IRenderer() = default;
      virtual ~IRenderer() = default;

      IRenderer(const IRenderer&) = default;
      IRenderer(IRenderer&&) = delete;
      auto operator=(const IRenderer&) -> IRenderer& = default;
      auto operator=(IRenderer&&) -> IRenderer& = delete;
   };
}
