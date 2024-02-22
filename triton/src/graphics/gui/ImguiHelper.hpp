#pragma once

namespace Triton::Graphics {
   class GraphicsDevice;
}

namespace Triton::Graphics::Gui {
   class ImGuiHelper {
    public:
      ImGuiHelper(const GraphicsDevice& graphicsDevice, GLFWwindow* window);
      ~ImGuiHelper();

      ImGuiHelper(const ImGuiHelper&) = delete;
      ImGuiHelper(ImGuiHelper&&) = delete;
      ImGuiHelper& operator=(const ImGuiHelper&) = delete;
      ImGuiHelper& operator=(ImGuiHelper&&) = delete;

    private:
      std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;
   };
}