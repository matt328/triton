#pragma once

#include "RenderObject.hpp"
#include "ResourceFactory.hpp"
#include "TransferData.hpp"
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
      void registerRenderObjectProvider(
          std::function<std::vector<RenderObject>()> renderObjectProvider);

      void registerPerFrameDataProvider(std::function<PerFrameData()> perFrameDataProvider);

      [[nodiscard]] const std::tuple<int, int> getWindowSize() const;

    private:
      class ContextImpl;
      std::unique_ptr<ContextImpl> impl;
      std::function<std::vector<RenderObject>()> renderObjectProvider;
      std::function<PerFrameData()> perFrameDataProvider;
   };
}