#pragma once

namespace tr {

struct ImageTransitionInfo {
  vk::Image image;
  vk::ImageLayout oldLayout;
  vk::ImageLayout newLayout;
  vk::ImageSubresourceRange subresourceRange;
};

class ImageTransitionQueue {
public:
  ImageTransitionQueue() = default;
  ~ImageTransitionQueue() = default;

  ImageTransitionQueue(const ImageTransitionQueue&) = delete;
  ImageTransitionQueue(ImageTransitionQueue&&) = delete;
  auto operator=(const ImageTransitionQueue&) -> ImageTransitionQueue& = delete;
  auto operator=(ImageTransitionQueue&&) -> ImageTransitionQueue& = delete;

  auto enqueue(std::vector<ImageTransitionInfo> imageTransition) -> void;
  auto dequeue() -> std::vector<ImageTransitionInfo>;

private:
  moodycamel::ReaderWriterQueue<std::vector<ImageTransitionInfo>> queue{10};
};

}
