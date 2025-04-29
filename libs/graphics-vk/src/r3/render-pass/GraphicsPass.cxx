#include "GraphicsPass.hpp"

namespace tr {
GraphicsPass::GraphicsPass(GraphicsPassConfig&& config,
                           std::shared_ptr<ImageManager> newImageManager)
    : imageManager{std::move(newImageManager)}, passConfig{std::move(config)} {
}

auto GraphicsPass::bind(const Frame* frame,
                        vk::raii::CommandBuffer& cmdBuffer,
                        PushConstantsBindFn& bindFn) -> void {
}

auto GraphicsPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
}

}
