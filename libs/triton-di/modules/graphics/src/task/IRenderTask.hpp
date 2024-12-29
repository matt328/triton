#pragma once

namespace tr {
class IRenderTask {
public:
  IRenderTask() = default;
  virtual ~IRenderTask() = default;

  IRenderTask(const IRenderTask&) = delete;
  IRenderTask(IRenderTask&&) = delete;
  auto operator=(const IRenderTask&) -> IRenderTask& = delete;
  auto operator=(IRenderTask&&) -> IRenderTask& = delete;

  virtual auto record(vk::raii::CommandBuffer& commandBuffer) -> void = 0;

  [[nodiscard]] auto getName() -> std::string_view;

private:
  std::string name;
};
}
