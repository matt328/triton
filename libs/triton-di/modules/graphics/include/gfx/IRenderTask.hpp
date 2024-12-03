#pragma once

namespace tr::gfx::task {
   class IRenderTask {
    public:
      IRenderTask() = default;
      virtual ~IRenderTask() = default;

      IRenderTask(const IRenderTask&) = delete;
      IRenderTask(IRenderTask&&) = delete;
      auto operator=(const IRenderTask&) -> IRenderTask& = delete;
      auto operator=(IRenderTask&&) -> IRenderTask& = delete;

      virtual auto record(vk::raii::CommandBuffer& commandBuffer) -> void = 0;
   };
}
