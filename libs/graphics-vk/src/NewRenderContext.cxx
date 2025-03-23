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

void NewRenderContext::update() {
  /*
    DebugMeshService -
      Check to see if any DebugMeshes are in the UploadComplete state
      If so, move them into the Renderable state

      If the list of Meshes in the Renderable state has changed since last frame,
      add their data to the renderData

      From here, the DefaultRenderScheduler should pick up the renderData and update buffers, etc
      maybe factor this out into separate buffer updaters for each render task?
  */
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
