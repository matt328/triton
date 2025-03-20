#pragma once

#include "fx/ITaskQueue.hpp"

#include <queue> // This should come from the precompiled headers but it isn't probably because the build is broke rn

namespace tr {

constexpr size_t DefaultMaxQueueSize = 2;

struct TaskQueueConfig {
  size_t maxQueueSize = 2;
};

class TaskQueue : public ITaskQueue {
public:
  explicit TaskQueue(size_t maxQueueSize) : maxQueueSize(maxQueueSize) {
    thread = std::thread([this]() { this->worker(); });
  }

  ~TaskQueue() override {
    {
      std::unique_lock<std::mutex> lock(mtx);
      stopFlag = true;
    }
    cv.notify_all();
    thread.join();
  }

  TaskQueue(const TaskQueue&) = delete;
  TaskQueue(TaskQueue&&) = delete;
  auto operator=(const TaskQueue&) -> TaskQueue& = delete;
  auto operator=(TaskQueue&&) -> TaskQueue& = delete;

protected:
  auto enqueueImpl(std::function<void()> task,
                   std::function<void()> onComplete) -> std::future<void> override {
    auto packagedTask = std::make_shared<std::packaged_task<void()>>(std::move(task));
    std::future<void> result = packagedTask->get_future();

    {
      std::unique_lock<std::mutex> lock(mtx);
      if (internalQueue.size() >= maxQueueSize) {
        cv.wait(lock, [this] { return internalQueue.size() < maxQueueSize || stopFlag; });
      }
      if (stopFlag) {
        throw std::runtime_error("enqueue on stopped TaskQueue");
      }

      internalQueue.emplace([this, packagedTask, onComplete = std::move(onComplete)]() mutable {
        (*packagedTask)();
        enqueueOnCallerThread(std::move(onComplete));
      });
    }

    cv.notify_one();
    return result;
  }

private:
  std::thread thread;
  std::mutex mtx;
  std::condition_variable cv;
  std::queue<std::function<void()>> internalQueue;

  std::mutex completeMtx;
  std::queue<std::function<void()>> completeQueue;

  bool stopFlag{};
  size_t maxQueueSize;

  void worker() {
    while (true) {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this]() { return !internalQueue.empty() || stopFlag; });

      if (stopFlag && internalQueue.empty()) {
        return;
      }

      auto task = std::move(internalQueue.front());
      internalQueue.pop();
      lock.unlock();
      task();
    }
  }

  void enqueueOnCallerThread(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(completeMtx);
    completeQueue.emplace(std::move(task));
  }
};

}
