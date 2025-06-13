#include "OrderedFrameGraph.hpp"
#include "r3/graph/BarrierGenerator.hpp"
#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

auto OrderedFrameGraph::addPass(std::unique_ptr<IRenderPass>&& pass) -> void {
  const auto size = renderPasses.size();
  renderPasses.push_back(std::move(pass));
  passesById.emplace(pass->getId(), size);
}

auto OrderedFrameGraph::getPass(PassId id) -> std::unique_ptr<IRenderPass>& {
  assert(passesById.contains(id));
  return renderPasses[passesById.at(id)];
}

auto OrderedFrameGraph::bake() -> void {
  auto barrierGenerator = BarrierGenerator{};
  barrierPlan = std::make_unique<BarrierPlan>(barrierGenerator.build(renderPasses));
}

auto OrderedFrameGraph::execute(const Frame* frame) -> FrameGraphResult {
  auto result = FrameGraphResult{};

  return result;
}

}
