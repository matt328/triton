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
               Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using R = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<R> result = task->get_future();

    enqueueImpl([task]() { (*task)(); }, // Execute the task
                [task, onComplete = std::forward<OnComplete>(onComplete)]() mutable {
                  if constexpr (!std::is_void_v<R>) {
                    onComplete(task->get_future().get()); // Pass result if non-void
                  } else {
                    onComplete(); // Call directly if void
                  }
                });

    return result;
  }

protected:
  virtual auto enqueueImpl(std::function<void()> task,
                           std::function<void()> onComplete) -> void = 0;
};

}
