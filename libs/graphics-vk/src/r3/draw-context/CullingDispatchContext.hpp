#pragma once

#include "IDispatchContext.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/draw-context/DispatchCreateInfos.hpp"

namespace tr {

class CullingDispatchContext : public IDispatchContext {
public:
  CullingDispatchContext(ContextId newId,
                         std::shared_ptr<BufferSystem> newBufferSystem,
                         CullingDispatchContextCreateInfo newCreateInfo);
  ~CullingDispatchContext() override = default;

  CullingDispatchContext(const CullingDispatchContext&) = default;
  CullingDispatchContext(CullingDispatchContext&&) = delete;
  auto operator=(const CullingDispatchContext&) -> CullingDispatchContext& = default;
  auto operator=(CullingDispatchContext&&) -> CullingDispatchContext& = delete;

  auto bind(const Frame* frame,
            vk::raii::CommandBuffer& commandBuffer,
            const vk::raii::PipelineLayout& layout) -> void override;

  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void override;

  auto getPushConstantSize() -> size_t override;

  [[nodiscard]] auto getGraphInfo() const -> PassGraphInfo override;

private:
  struct PushConstants {
    uint64_t resourceTableAddress;
    uint64_t frameDataAddress;
  };

  CullingDispatchContextCreateInfo createInfo;
};

}
