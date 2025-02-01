#pragma once

#include "cm/Handles.hpp"

namespace tr {

// Maximum size of the GpuObjectData buffer array
constexpr auto MAX_OBJECTS = 128;
constexpr auto ALIGNMENT = 16;

struct alignas(ALIGNMENT) GpuObjectData {
  glm::mat4 model{};
  TextureHandle textureId{};
  uint32_t animationDataIndex{};
  uint32_t padding[4]{};
};

}
