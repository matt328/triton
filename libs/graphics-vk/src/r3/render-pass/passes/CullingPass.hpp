#pragma once

#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

class ContextFactory;
class PipelineFactory;

class CullingPass : public IRenderPass {
public:
  explicit CullingPass(std::shared_ptr<ContextFactory> newContextFactory,
                       std::shared_ptr<PipelineFactory> newPipelineFactory,
                       PassId newPassId);
  ~CullingPass() override = default;

  CullingPass(const CullingPass&) = delete;
  CullingPass(CullingPass&&) = delete;
  auto operator=(const CullingPass&) -> CullingPass& = delete;
  auto operator=(CullingPass&&) -> CullingPass& = delete;

  [[nodiscard]] auto getId() const -> PassId override;
  auto execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void override;
  auto registerDispatchContext(Handle<IDispatchContext> handle) -> void override;
  [[nodiscard]] auto getGraphInfo() const -> PassGraphInfo override;

private:
  std::shared_ptr<ContextFactory> contextFactory;
  std::shared_ptr<PipelineFactory> pipelineFactory;

  PassId id;
  std::vector<Handle<IDispatchContext>> dispatchableContexts;

  std::optional<vk::raii::Pipeline> pipeline;
  std::optional<vk::raii::PipelineLayout> pipelineLayout;
};

}
