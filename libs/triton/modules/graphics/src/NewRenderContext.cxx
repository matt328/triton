#include "NewRenderContext.hpp"

namespace tr {

NewRenderContext::NewRenderContext(std::shared_ptr<IFrameManager> newFrameManager,
                                   std::shared_ptr<IRenderScheduler> newRenderScheduler)
    : frameManager{std::move(newFrameManager)}, renderScheduler{std::move(newRenderScheduler)} {
  Log.trace("Creating NewRenderContext");
}

NewRenderContext::~NewRenderContext() {
  Log.trace("Destroying NewRenderContext");
}

void NewRenderContext::updateTerrainSystem() {
}

/// This gets called directly after setRenderData
void NewRenderContext::renderNextFrame() {

  {
    const auto result = frameManager->acquireFrame();

    if (std::holds_alternative<Frame*>(result)) {
      auto* frame = std::get<Frame*>(result);
      { renderScheduler->updatePerFrameRenderData(frame, renderData); }
      {
        renderScheduler->recordRenderTasks(frame,
                                           !renderData.staticGpuMeshData.empty() ||
                                               !renderData.dynamicMeshData.empty() ||
                                               !renderData.terrainMeshData.empty());
      }
      { renderScheduler->endFrame(frame); }
      return;
    }

    if (const auto acquireResult = std::get<ImageAcquireResult>(result);
        acquireResult == ImageAcquireResult::Error) {
      Log.warn("Failed to acquire swapchain image");
    }
  }
}

void NewRenderContext::waitIdle() {
  Log.trace("waitIdle");
}

void NewRenderContext::setRenderData(const RenderData& newRenderData) {
  auto lock = std::lock_guard{renderDataMutex};
  LockableName(renderDataMutex, "SetRenderData", 13);
  LockMark(renderDataMutex);
  renderData = newRenderData;
}

}
