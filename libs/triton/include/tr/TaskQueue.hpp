#pragma once

#include <mutex>
#include <thread>
#include <future>

namespace tr::util {

   constexpr size_t DefaultMaxQueueSize = 10;

   class TaskQueue {
    public:
      TaskQueue(size_t maxQueueSize = DefaultMaxQueueSize) : maxQueueSize{maxQueueSize} {
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
      TaskQueue& operator=(const TaskQueue&) = delete;
      TaskQueue& operator=(TaskQueue&&) = delete;

      template <class F, class... Args>
      auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F&&, Args&&...>> {
         using ReturnType = std::invoke_result_t<F&&, Args&&...>;

         auto task = std::make_shared<std::packaged_task<ReturnType()>>(
             std::bind(std::forward<F>(f), std::forward<Args>(args)...));

         std::future<ReturnType> res = task->get_future();

         std::unique_lock<LockableBase(std::mutex)> lock(mtx);
         LockMark(mtx);
         if (taskQueue.size() >= maxQueueSize) {
            cv.wait(lock, [this] { return taskQueue.size() < maxQueueSize || stopFlag; });
         }

         if (stopFlag) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
         }

         taskQueue.emplace([task]() { (*task)(); });
         cv.notify_one();

         return res;
      }

    private:
      std::thread thread;
      TracyLockableN(std::mutex, mtx, "TaskQueueLock");
      std::condition_variable_any cv;
      std::queue<std::function<void()>> taskQueue;

      bool stopFlag{};
      size_t maxQueueSize;

      void worker() {
         while (true) {
            ZoneNamedN(v2, "TaskQueue Worker", true);
            std::unique_lock<LockableBase(std::mutex)> lock(mtx);
            LockMark(mtx);
            cv.wait(lock, [this]() { return !taskQueue.empty() || stopFlag; });

            if (stopFlag && taskQueue.empty()) {
               return;
            }

            auto task = std::move(taskQueue.front());
            taskQueue.pop();
            lock.unlock();

            {
               ZoneNamedN(v3, "Task", true);
               task();
            }
         }
      }
   };
}