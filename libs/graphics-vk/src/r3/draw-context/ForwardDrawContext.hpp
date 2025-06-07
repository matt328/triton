#pragma once

#include "IDispatchContext.hpp"
#include "r3/draw-context/DispatchCreateInfos.hpp"

namespace tr {

class ForwardDrawContext : public IDispatchContext {
public:
  ForwardDrawContext(ContextId newId,
                     std::shared_ptr<BufferSystem> newBufferSystem,
                     ForwardDrawContextCreateInfo newCreateInfo);
  ~ForwardDrawContext() override = default;

  ForwardDrawContext(const ForwardDrawContext&) = default;
  ForwardDrawContext(ForwardDrawContext&&) = delete;
  auto operator=(const ForwardDrawContext&) -> ForwardDrawContext& = default;
  auto operator=(ForwardDrawContext&&) -> ForwardDrawContext& = delete;

  auto bind(const Frame* frame,
            vk::raii::CommandBuffer& commandBuffer,
            const vk::raii::PipelineLayout& layout) -> void override;

  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void override;

  auto getPushConstantSize() -> size_t override;

private:
  ForwardDrawContextCreateInfo createInfo;
};

}
