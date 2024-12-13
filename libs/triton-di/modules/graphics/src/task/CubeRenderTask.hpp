#pragma once

#include "IRenderTask.hpp"

namespace tr::gfx::task {

   class CubeRenderTask final : public IRenderTask {
    public:
      CubeRenderTask() = default;
      ~CubeRenderTask() override = default;
      CubeRenderTask(const CubeRenderTask&) = delete;
      CubeRenderTask(CubeRenderTask&&) = delete;
      auto operator=(const CubeRenderTask&) -> CubeRenderTask& = delete;
      auto operator=(CubeRenderTask&&) -> CubeRenderTask& = delete;

      auto record(vk::raii::CommandBuffer& commandBuffer) -> void override;
   };

}
