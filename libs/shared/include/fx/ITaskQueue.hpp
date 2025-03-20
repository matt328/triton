#pragma once

namespace tr {

class ITaskQueue {
public:
  ITaskQueue() = default;
  virtual ~ITaskQueue() = default;

  ITaskQueue(const ITaskQueue&) = delete;
  ITaskQueue(ITaskQueue&&) = delete;
  auto operator=(const ITaskQueue&) -> ITaskQueue& = delete;
  auto operator=(ITaskQueue&&) -> ITaskQueue& = delete;

  template <class F, class... Args, class OnComplete>
  auto enqueue(F&& f,
               OnComplete&& onComplete,
               Args&&... args) -> std::future<std::invoke_result_t<F&&, Args&&...>> {
    return enqueueImpl(std::bind(std::forward<F>(f), std::forward<Args>(args)...),
                       std::forward<OnComplete>(onComplete));
  }

protected:
  virtual auto enqueueImpl(std::function<void()> task,
                           std::function<void()> onComplete) -> std::future<void> = 0;
};

}
