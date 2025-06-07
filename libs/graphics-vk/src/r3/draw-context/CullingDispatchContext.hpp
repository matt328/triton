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

private:
  struct PushConstants {
    uint32_t objectCount;
    uint64_t objectDataAddress;
    uint64_t objectPositionsAddress;
    uint64_t objectRotationsAddress;
    uint64_t objectScalesAddress;

    uint64_t outputIndirectCommandAddress;
    uint64_t outputIndirectCountAddress;
    uint64_t geometryRegionAddress;
    uint64_t indexDataAddress;
    uint64_t vertexPositionAddress;
    uint64_t vertexNormalAddress;
    uint64_t vertexTexCoordAddress;
    uint64_t vertexColorAddress;
  };

  CullingDispatchContextCreateInfo createInfo;
};

}
