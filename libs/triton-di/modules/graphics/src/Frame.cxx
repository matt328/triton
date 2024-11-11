#include "Frame.hpp"

namespace tr::gfx {
   Frame::Frame(std::shared_ptr<IGraphicsDevice> graphicsDevice,
                std::shared_ptr<vk::raii::ImageView> depthImageView,
                std::shared_ptr<sb::IShaderBindingFactory> shaderBindingFactory,
                std::string_view name) {
   }

   Frame::~Frame() {
   }

   void Frame::registerStorageBuffer(const std::string& name, size_t size) {
   }

   void Frame::destroySwapchainResources() {
   }

   void Frame::createSwapchainResources() {
   }
}