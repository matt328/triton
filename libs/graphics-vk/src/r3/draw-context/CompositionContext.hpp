#pragma once

#include "IDispatchContext.hpp"
#include "r3/draw-context/DispatchCreateInfos.hpp"

namespace tr {

class IShaderBindingFactory;

class CompositionContext : public IDispatchContext {
public:
  CompositionContext(ContextId newId,
                     std::shared_ptr<BufferSystem> newBufferSystem,
                     std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                     CompositionContextCreateInfo newCreateInfo);
  ~CompositionContext() = default;

  CompositionContext(const CompositionContext&) = default;
  CompositionContext(CompositionContext&&) = delete;
  auto operator=(const CompositionContext&) -> CompositionContext& = default;
  auto operator=(CompositionContext&&) -> CompositionContext& = delete;

  auto bind(const Frame* frame,
            vk::raii::CommandBuffer& commandBuffer,
            const vk::raii::PipelineLayout& layout) -> void override;

  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void override;

  auto getPushConstantSize() -> size_t override;

  [[nodiscard]] auto getGraphInfo() const -> PassGraphInfo override;

private:
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  CompositionContextCreateInfo createInfo;
};

}
