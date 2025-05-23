#include "GeometryAllocator.hpp"
#include "resources/allocators/AnimationAllocator.hpp"
#include "resources/allocators/ColorAllocator.hpp"
#include "resources/allocators/IndexAllocator.hpp"
#include "resources/allocators/NormalAllocator.hpp"
#include "resources/allocators/PositionAllocator.hpp"
#include "resources/allocators/TexCoordAllocator.hpp"

namespace tr {

GeometryAllocator::GeometryAllocator(const GeometryBuffers& geometryBuffers) {
  indexAllocator = std::make_unique<IndexAllocator>(geometryBuffers.indexBuffer);
  positionAllocator = std::make_unique<PositionAllocator>(geometryBuffers.positionBuffer);
  colorAllocator = std::make_unique<ColorAllocator>(geometryBuffers.colorBuffer);
  texCoordAllocator = std::make_unique<TexCoordAllocator>(geometryBuffers.texCoordBuffer);
  normalAllocator = std::make_unique<NormalAllocator>(geometryBuffers.normalBuffer);
  animationAllocator = std::make_unique<AnimationAllocator>(geometryBuffers.animationBuffer);
}

auto GeometryAllocator::allocate(const UploadRequestData& requestData)
    -> std::optional<MeshBufferRegion> {

  auto visitor = [&](auto&& arg) -> std::optional<MeshBufferRegion> {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, UploadGeometryRequest>) {
      return handleUploadGeometryRequest(requestData.resourceId,
                                         requestData.currentStagingOffset,
                                         arg);
    }
    Log.warn("GeometryAllocator can only handle UploadGeometryRequests");
    return std::nullopt;
  };
  return std::visit(visitor, requestData.requestVariant);
}

auto GeometryAllocator::handleUploadGeometryRequest(size_t resourceId,
                                                    size_t stagingOffset,
                                                    const UploadGeometryRequest& g)
    -> std::optional<MeshBufferRegion> {
  MeshBufferRegion region;
  region.resourceId = resourceId;

  if (!g.data->indexData.empty()) {
    auto indexRegion = indexAllocator->allocate(
        UploadRequestData{.resourceId = resourceId,
                          .currentStagingOffset = stagingOffset,
                          .requestVariant = IndexDataRequest{.indexData = g.data->indexData}});

    if (indexRegion) {
      region.regions.push_back((*indexRegion).regions.front());
    }
  }

  if (!g.data->positionData.empty()) {
    auto positionRegion = positionAllocator->allocate(UploadRequestData{
        .resourceId = resourceId,
        .currentStagingOffset = stagingOffset,
        .requestVariant = PositionDataRequest{.positionData = g.data->positionData}});
    if (positionRegion) {
      region.regions.push_back((*positionRegion).regions.front());
    }
  }

  if (!g.data->colorData.empty()) {
    auto colorRegion = colorAllocator->allocate(
        UploadRequestData{.resourceId = resourceId,
                          .currentStagingOffset = stagingOffset,
                          .requestVariant = ColorDataRequest{.colorData = g.data->colorData}});
    if (colorRegion) {
      region.regions.push_back((*colorRegion).regions.front());
    }
  }

  if (!g.data->texCoordData.empty()) {
    auto texCoordRegion = texCoordAllocator->allocate(UploadRequestData{
        .resourceId = resourceId,
        .currentStagingOffset = stagingOffset,
        .requestVariant = TexCoordDataRequest{.texCoordData = g.data->texCoordData}});
    if (texCoordRegion) {
      region.regions.push_back((*texCoordRegion).regions.front());
    }
  }

  if (!g.data->normalData.empty()) {
    auto normalRegion = normalAllocator->allocate(
        UploadRequestData{.resourceId = resourceId,
                          .currentStagingOffset = stagingOffset,
                          .requestVariant = NormalDataRequest{.normalData = g.data->normalData}});
    if (normalRegion) {
      region.regions.push_back((*normalRegion).regions.front());
    }
  }

  if (!g.data->animationData.empty()) {
    auto animationRegion = animationAllocator->allocate(UploadRequestData{
        .resourceId = resourceId,
        .currentStagingOffset = stagingOffset,
        .requestVariant = AnimationDataRequest{.animationData = g.data->animationData}});
    if (animationRegion) {
      region.regions.push_back((*animationRegion).regions.front());
    }
  }

  return std::make_optional<MeshBufferRegion>(region);
}

auto GeometryAllocator::reset() -> void {
  indexAllocator->reset();
  positionAllocator->reset();
  colorAllocator->reset();
  texCoordAllocator->reset();
  normalAllocator->reset();
  animationAllocator->reset();
}

}
