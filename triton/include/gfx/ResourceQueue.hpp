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

      std::future<gfx::MeshMaterialHandle> addGltf(const std::filesystem::path& path);

    private:
      std::thread thread;
      std::mutex mtx;
      std::condition_variable cv;
      std::queue<std::packaged_task<gfx::MeshMaterialHandle()>> taskQueue;

      bool stopFlag{};

      void worker();

      gfx::MeshMaterialHandle loadGltf(const std::filesystem::path& path);
   };
}