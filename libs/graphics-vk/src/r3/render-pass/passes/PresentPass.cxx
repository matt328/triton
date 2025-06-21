#include "PresentPass.hpp"

namespace tr {

PresentPass::PresentPass(PassId newPassId) : passId{newPassId} {
}

[[nodiscard]] auto PresentPass::getId() const -> PassId {
  return passId;
}

auto PresentPass::execute([[maybe_unused]] Frame* frame,
                          [[maybe_unused]] vk::raii::CommandBuffer& cmdBuffer) -> void {
  // NOOP
}

auto PresentPass::registerDispatchContext([[maybe_unused]] Handle<IDispatchContext> handle)
    -> void {
  // NOOP
}

[[nodiscard]] auto PresentPass::getGraphInfo() const -> PassGraphInfo {
  return PassGraphInfo{.imageReads = {{
                           .alias = ImageAlias::SwapchainImage,
                           .accessFlags = vk::AccessFlagBits2::eNone,
                           .stageFlags = vk::PipelineStageFlagBits2::eBottomOfPipe,
                           .aspectFlags = vk::ImageAspectFlagBits::eColor,
                           .layout = vk::ImageLayout::ePresentSrcKHR,
                       }}};
}

}
