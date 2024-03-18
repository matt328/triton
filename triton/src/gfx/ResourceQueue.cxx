#include "gfx/ResourceQueue.hpp"
#include <future>

namespace tr::gfx {
   ResourceQueue::ResourceQueue() {
      thread = std::thread([this]() { this->worker(); });
   }

   ResourceQueue::~ResourceQueue() {
      std::unique_lock<std::mutex> lock(mtx);
      stopFlag = true;
      lock.unlock();
      cv.notify_all();
      thread.join();
   }

   std::future<gfx::MeshMaterialHandle> ResourceQueue::addGltf(const std::filesystem::path& path) {
      auto task = std::packaged_task<gfx::MeshMaterialHandle()>(
          [path, this]() { return this->loadGltf(path); });

      auto f = task.get_future();

      {
         std::lock_guard<std::mutex> lock(mtx);
         taskQueue.push(std::move(task));
      }

      cv.notify_one();

      return f;
   }

   gfx::MeshMaterialHandle ResourceQueue::loadGltf(const std::filesystem::path& path) {
      ZoneNamedN(var, "loadGltf", true);
      return static_cast<gfx::MeshMaterialHandle>(3);
   }

   void ResourceQueue::worker() {
      Log::debug << "worker starting" << std::endl;
      while (true) {
         ZoneNamedN(v2, "worker", true);
         std::unique_lock<std::mutex> lock(mtx);
         cv.wait(lock, [this]() { return !taskQueue.empty() || stopFlag; });

         if (stopFlag && taskQueue.empty()) {
            return;
         }

         auto task = std::move(taskQueue.front());
         taskQueue.pop();

         lock.unlock();

         {
            ZoneNamedN(v3, "execute task", true);
            task();
         }
      }
   }
}