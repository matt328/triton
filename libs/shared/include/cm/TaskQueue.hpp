#pragma once

namespace tr {

constexpr size_t DefaultMaxQueueSize = 2;

struct TaskQueueConfig {
  size_t maxQueueSize = 2;
};

class TaskQueue {
public:
  explicit TaskQueue(const TaskQueueConfig& config) : maxQueueSize{config.maxQueueSize} {
    thread = std::thread([this]() { this->worker(); });
  }

  ~TaskQueue() {
    std::unique_lock<LockableBase(std::mutex)> lock(mtx);
    stopFlag = true;
    lock.unlock();
    cv.notify_all();
    thread.join();
  }

  TaskQueue(const TaskQueue&) = delete;
  TaskQueue(TaskQueue&&) = delete;
  auto operator=(const TaskQueue&) -> TaskQueue& = delete;
  auto operator=(TaskQueue&&) -> TaskQueue& = delete;

  template <class F, class... Args, class OnComplete>
  auto enqueue(F&& f,
               OnComplete&& onComplete,
               Args&&... args) -> std::future<std::invoke_result_t<F&&, Args&&...>> {
    using ReturnType = std::invoke_result_t<F&&, Args&&...>;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<ReturnType> res = task->get_future();
    auto resPtr = std::make_shared<std::future<ReturnType>>(std::move(res));

    {
      std::unique_lock<LockableBase(std::mutex)> lock(mtx);
      LockMark(mtx);

      if (internalQueue.size() >= maxQueueSize) {
        cv.wait(lock, [this] { return internalQueue.size() < maxQueueSize || stopFlag; });
      }

      if (stopFlag) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
      }

      internalQueue.emplace(
          [this, task, resPtr, onComplete = std::forward<OnComplete>(onComplete)]() mutable {
            // Execute the task
            (*task)();
            if constexpr (!std::is_void_v<ReturnType>) {
              ReturnType result = resPtr->get(); // Get the result
              enqueueOnCallerThread([onComplete = std::move(onComplete),
                                     result = std::move(result)]() mutable { onComplete(result); });
            } else {
              resPtr->get();
              enqueueOnCallerThread(std::move(onComplete));
            }
          });
    }

    cv.notify_one();
    return res;
  }

  void processCompleteTasks() {
    std::unique_lock<LockableBase(std::mutex)> lock(completeMtx);
    while (!completeQueue.empty()) {
      auto task = std::move(completeQueue.front());
      completeQueue.pop();
      lock.unlock();
      task();
      lock.lock();
    }
  }

private:
  std::thread thread;
  TracyLockableN(std::mutex, mtx, "TaskQueueLock");
  std::condition_variable_any cv;
  std::queue<std::function<void()>> internalQueue;

  TracyLockableN(std::mutex, completeMtx, "CompleteQueueLock");
  std::queue<std::function<void()>> completeQueue;

  bool stopFlag{};
  size_t maxQueueSize;

  void worker() {
    while (true) {
      ZoneNamedN(v2, "TaskQueue Worker", true);
      std::unique_lock<LockableBase(std::mutex)> lock(mtx);
      LockMark(mtx);
      cv.wait(lock, [this]() { return !internalQueue.empty() || stopFlag; });

      if (stopFlag && internalQueue.empty()) {
        return;
      }

      auto task = std::move(internalQueue.front());
      internalQueue.pop();
      lock.unlock();

      {
        ZoneNamedN(v3, "Task", true);
        Log.debug("Executing Task");
        task();
      }
    }
  }

  void enqueueOnCallerThread(std::function<void()> task) {
    std::unique_lock<LockableBase(std::mutex)> lock(completeMtx);
    completeQueue.emplace(std::move(task));
  }
};
}
