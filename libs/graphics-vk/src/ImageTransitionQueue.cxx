#include "ImageTransitionQueue.hpp"

namespace tr {

auto ImageTransitionQueue::enqueue(std::vector<ImageTransitionInfo> imageTransition) -> void {
  queue.enqueue(std::move(imageTransition));
}

auto ImageTransitionQueue::dequeue() -> std::vector<ImageTransitionInfo> {
  auto batch = std::vector<ImageTransitionInfo>{};
  queue.try_dequeue(batch);
  return batch;
}

}
