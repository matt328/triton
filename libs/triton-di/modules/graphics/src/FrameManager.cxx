#include "FrameManager.hpp"

#include "Frame.hpp"

namespace tr {
FrameManager::FrameManager(size_t numFrames,
                           const std::shared_ptr<IGraphicsDevice>& graphicsDevice,
                           const std::shared_ptr<vk::raii::ImageView>& depthImageView,
                           const std::shared_ptr<IShaderBindingFactory>& sbFactory)
    : numFrames(numFrames) {
  for (size_t i = 0; i < numFrames; ++i) {
    auto name = std::stringstream{};
    name << "Frame " << i;
    frames.push_back(
        std::make_unique<Frame>(graphicsDevice, depthImageView, sbFactory, name.str()));
  }
}

FrameManager::~FrameManager() {
  Log.trace("Destroying FrameManager");
}

[[nodiscard]] auto FrameManager::getCurrentFrame() const -> Frame& {
  return *frames[currentFrame];
}

void FrameManager::nextFrame() {
  currentFrame = (currentFrame + 1) % numFrames;
}

void FrameManager::registerStorageBuffer(const std::string& name, size_t size) const {
  for (const auto& frame : frames) {
    frame->registerStorageBuffer(name, size);
  }
}

void FrameManager::destroySwapchainResources() {
  for (const auto& frame : frames) {
    frame->destroySwapchainResources();
  }
}

void FrameManager::createSwapchainResources() {
  for (const auto& frame : frames) {
    frame->createSwapchainResources();
  }
}

}
