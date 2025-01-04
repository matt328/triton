#pragma once
#include <gfx/IRenderContext.hpp>
#include <gfx/IRenderScheduler.hpp>
#include <task/IFrameManager.hpp>

namespace tr {

class NewRenderContext final : public IRenderContext {
public:
  NewRenderContext(std::shared_ptr<IFrameManager> newFrameManager,
                   std::shared_ptr<IRenderScheduler> newRenderScheduler,
                   std::shared_ptr<queue::Graphics> newGraphicsQueue);
  ~NewRenderContext() override;

  NewRenderContext(const NewRenderContext&) = delete;
  NewRenderContext(NewRenderContext&&) = delete;
  auto operator=(const NewRenderContext&) -> NewRenderContext& = delete;
  auto operator=(NewRenderContext&&) -> NewRenderContext& = delete;

  void renderNextFrame() override;
  void waitIdle() override;
  void setRenderData(const RenderData& renderData) override;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<IRenderScheduler> renderScheduler;
  std::shared_ptr<queue::Graphics> graphicsQueue;

  mutable TracyLockable(std::mutex, renderDataMutex);
  RenderData renderData;
};

}
