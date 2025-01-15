#pragma once

#include "cm/RenderData.hpp"
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

  virtual auto updatePerFrameRenderData(Frame& frame, const RenderData& renderData) -> void = 0;

  virtual auto recordRenderTasks(Frame& frame, bool recordCommands) -> void = 0;

  virtual auto endFrame(Frame& frame) -> void = 0;

  virtual auto executeTasks(Frame& frame, bool recordTasks) const -> void = 0;
};

}
