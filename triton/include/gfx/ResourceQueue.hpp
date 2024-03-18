#pragma once

#include "gfx/Handles.hpp"
#include <condition_variable>
#include <future>

namespace tr::gfx {
   class ResourceQueue {
    public:
      ResourceQueue();
      ~ResourceQueue();

      ResourceQueue(const ResourceQueue&) = delete;
      ResourceQueue(ResourceQueue&&) = delete;
      ResourceQueue& operator=(const ResourceQueue&) = delete;
      ResourceQueue& operator=(ResourceQueue&&) = delete;

      // add new work item to the pool
      template <class F, class... Args>
      auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F&&, Args&&...>> {
         using return_type = std::invoke_result_t<F&&, Args&&...>;

         auto task = std::make_shared<std::packaged_task<return_type()>>(
             std::bind(std::forward<F>(f), std::forward<Args>(args)...));

         std::future<return_type> res = task->get_future();

         std::unique_lock<std::mutex> lock(mtx);
         // if (tasks.size() >= max_queue_size)
         //    // wait for the queue to empty or be stopped
         //    condition_producers.wait(lock,
         //                             [this] { return tasks.size() < max_queue_size || stop; });

         // don't allow enqueueing after stopping the pool
         if (stopFlag)
            throw std::runtime_error("enqueue on stopped ThreadPool");

         taskQueue.emplace([task]() { (*task)(); });
         cv.notify_one();

         return res;
      }

    private:
      std::thread thread;
      std::mutex mtx;
      std::condition_variable cv;
      std::queue<std::function<void()>> taskQueue;

      bool stopFlag{};

      void worker();

      gfx::MeshMaterialHandle loadGltf(const std::filesystem::path& path);
   };
}