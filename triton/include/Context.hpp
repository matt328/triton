#pragma once

#include "ResourceFactory.hpp"
#include <string_view>

class RenderSystem;
class IGame;

namespace graphics {
   class Context : public IResourceFactory {
    public:
      Context(GLFWwindow* window);
      ~Context() override;

      Context(const Context&) = delete;
      Context(Context&&) = delete;
      Context& operator=(const Context&) = delete;
      Context& operator=(Context&&) = delete;

      void render();
      void waitIdle();
      void windowResized(const int height, const int width);

      std::string createMesh(const std::string_view& filename) override;
      uint32_t createTexture(const std::string_view& filename) override;

      void registerGame(std::shared_ptr<IGame> game);

      [[nodiscard]] const std::tuple<int, int> getWindowSize() const;

    private:
      class ContextImpl;
      std::unique_ptr<ContextImpl> impl;
   };
}