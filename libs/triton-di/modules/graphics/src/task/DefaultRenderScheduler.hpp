#pragma once

#include "CommandBufferManager.hpp"
#include "IFrameManager.hpp"
#include "gfx/IRenderScheduler.hpp"
#include "IRenderTask.hpp"

namespace tr::gfx {

   class DefaultRenderScheduler final : public task::IRenderScheduler {
    public:
      explicit DefaultRenderScheduler(std::shared_ptr<task::IFrameManager> newFrameManager,
                                      std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                      std::shared_ptr<queue::Graphics> newGraphicsQueue);
      ~DefaultRenderScheduler() override;

      DefaultRenderScheduler(const DefaultRenderScheduler&) = delete;
      DefaultRenderScheduler(DefaultRenderScheduler&&) = delete;
      auto operator=(const DefaultRenderScheduler&) -> DefaultRenderScheduler& = delete;
      auto operator=(DefaultRenderScheduler&&) -> DefaultRenderScheduler& = delete;

      auto executeStaticTasks(Frame& frame) const -> void override;
      auto addStaticTask(std::shared_ptr<task::IRenderTask> task) -> void override;
      auto recordRenderTasks(Frame& frame) const -> void override;
      auto setupCommandBuffersForFrame(Frame& frame) -> void override;
      auto endFrame(Frame& frame) const -> void override;

    private:
      std::shared_ptr<task::IFrameManager> frameManager;
      std::shared_ptr<CommandBufferManager> commandBufferManager;
      std::shared_ptr<queue::Graphics> graphicsQueue;

      std::vector<std::shared_ptr<task::IRenderTask>> staticRenderTasks;
   };

}
