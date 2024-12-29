#pragma once

namespace tr {

class IGraphicsDevice;
class IShaderBindingFactory;
class Frame;

class FrameManager {
public:
  explicit FrameManager(size_t numFrames,
                        const std::shared_ptr<IGraphicsDevice>& graphicsDevice,
                        const std::shared_ptr<vk::raii::ImageView>& depthImageView,
                        const std::shared_ptr<IShaderBindingFactory>& sbFactory);
  ~FrameManager();

  FrameManager(const FrameManager&) = default;
  FrameManager(FrameManager&&) = delete;
  auto operator=(const FrameManager&) -> FrameManager& = default;
  auto operator=(FrameManager&&) -> FrameManager& = delete;

  [[nodiscard]] auto getCurrentFrame() const -> Frame&;
  void nextFrame();
  void registerStorageBuffer(const std::string& name, size_t size) const;
  void destroySwapchainResources();
  void createSwapchainResources();
  [[nodiscard]] auto getFrames() const -> const std::vector<std::unique_ptr<Frame>>&;

private:
  size_t currentFrame = 0;
  size_t numFrames;
  std::vector<std::unique_ptr<Frame>> frames;
};

}
