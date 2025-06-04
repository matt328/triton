#include "DebugStateBuffer.hpp"

namespace tr {

DebugStateBuffer::DebugStateBuffer() : previous{SimState{2}}, next{SimState{2}} {
  previous.objectMetadata[0] = (GpuObjectData{.transformIndex = 0,
                                              .rotationIndex = 0,
                                              .scaleIndex = 0,
                                              .geometryRegionId = 0,
                                              .materialId = 0,
                                              .animationId = 0});
  previous.positions[0] = GpuTransformData{.position = glm::vec3(4.f, 5.f, 6.f)};
  previous.rotations[0] = GpuRotationData{.rotation = glm::identity<glm::quat>()};
  previous.scales[0] = GpuScaleData{.scale = glm::one<glm::vec3>()};

  previous.objectMetadata[1] = (GpuObjectData{.transformIndex = 1,
                                              .rotationIndex = 1,
                                              .scaleIndex = 1,
                                              .geometryRegionId = 3,
                                              .materialId = 3,
                                              .animationId = 3});
  previous.positions[1] = GpuTransformData{.position = glm::vec3(7.f, 8.f, 9.f)};
  previous.rotations[1] = GpuRotationData{.rotation = glm::quat(1.f, 2.f, 3.f, 5.f)};
  previous.scales[1] = GpuScaleData{.scale = glm::vec3(3.f, 3.f, 3.f)};
}

auto DebugStateBuffer::getStates(SimState& stateA,
                                 SimState& stateB,
                                 float& alpha,
                                 Timestamp currentTimeSec) -> bool {
  stateA = previous;
  stateB = next;
  alpha = constAlpha;
  return true;
}

auto DebugStateBuffer::getWriteSlot() -> SimState* {
  return &previous;
}

auto DebugStateBuffer::commitWrite() -> void {
}

}
