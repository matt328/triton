#pragma once

#include "gfx/IRenderScheduler.hpp"
#include "gfx/IRenderTask.hpp"

namespace tr::gfx {

   class DefaultRenderScheduler : public task::IRenderScheduler {
    public:
      DefaultRenderScheduler();
      ~DefaultRenderScheduler() override;

      DefaultRenderScheduler(const DefaultRenderScheduler&) = delete;
      DefaultRenderScheduler(DefaultRenderScheduler&&) = delete;
      auto operator=(const DefaultRenderScheduler&) -> DefaultRenderScheduler& = delete;
      auto operator=(DefaultRenderScheduler&&) -> DefaultRenderScheduler& = delete;
      auto execute(vk::raii::CommandBuffer& commandBuffer) const -> void override;
      auto addTask(std::shared_ptr<task::IRenderTask> task) -> void override;

    private:
      std::vector<std::shared_ptr<task::IRenderTask>> tasks;
   };

}
