#pragma once

#include "api/fx/ResourceEvents.hpp"

namespace tr {

struct IndexDataRequest {
  std::vector<GpuIndexData> indexData;
};

struct PositionDataRequest {
  std::vector<GpuVertexPositionData> positionData;
};

struct ColorDataRequest {
  std::vector<GpuVertexColorData> colorData;
};

struct TexCoordDataRequest {
  std::vector<GpuVertexTexCoordData> texCoordData;
};

struct NormalDataRequest {
  std::vector<GpuVertexNormalData> normalData;
};

struct AnimationDataRequest {
  std::vector<GpuAnimationData> animationData;
};

using UploadRequestVariant = std::variant<UploadGeometryRequest,
                                          IndexDataRequest,
                                          PositionDataRequest,
                                          ColorDataRequest,
                                          TexCoordDataRequest,
                                          NormalDataRequest,
                                          AnimationDataRequest>;

}
