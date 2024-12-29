#pragma once

#include "task/Frame.hpp"

namespace tr {

class IRenderScheduler {
public:
  IRenderScheduler() = default;
  virtual ~IRenderScheduler() = default;

  IRenderScheduler(const IRenderScheduler&) = delete;
  IRenderScheduler(IRenderScheduler&&) = delete;
  auto operator=(IRenderScheduler&&) -> IRenderScheduler& = delete;
  auto operator=(const IRenderScheduler&) -> IRenderScheduler& = delete;

  virtual auto setupCommandBuffersForFrame(Frame& frame) -> void = 0;

  virtual auto recordRenderTasks(Frame& frame) const -> void = 0;

  virtual auto endFrame(Frame& frame) const -> void = 0;

  virtual auto executeTasks(Frame& frame) const -> void = 0;
};

}
