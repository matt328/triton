#include <utility>

#include "RenderPass.hpp"

namespace tr {

RenderPass::RenderPass(RenderPassCreateInfo& createInfo) : debugName{std::move(createInfo.name)} {
}

[[nodiscard]] auto RenderPass::accepts([[maybe_unused]] const RenderConfig& config) const -> bool {
  return true;
}

auto RenderPass::addDrawContext([[maybe_unused]] RenderConfigHandle handle,
                                [[maybe_unused]] DrawContext* drawContext) -> void {
}

auto RenderPass::execute([[maybe_unused]] const Frame* frame,
                         [[maybe_unused]] vk::CommandBuffer& cmdBuffer) -> void {
}

}
