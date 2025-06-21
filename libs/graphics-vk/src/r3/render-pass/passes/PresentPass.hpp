#pragma once

#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

class PresentPass : public IRenderPass {
public:
  PresentPass(PassId newPassId);
  ~PresentPass() = default;

  PresentPass(const PresentPass&) = default;
  PresentPass(PresentPass&&) = delete;
  auto operator=(const PresentPass&) -> PresentPass& = default;
  auto operator=(PresentPass&&) -> PresentPass& = delete;

  [[nodiscard]] auto getId() const -> PassId override;
  auto execute(Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void override;
  auto registerDispatchContext(Handle<IDispatchContext> handle) -> void override;
  [[nodiscard]] auto getGraphInfo() const -> PassGraphInfo override;

private:
  PassId passId;
};

}
