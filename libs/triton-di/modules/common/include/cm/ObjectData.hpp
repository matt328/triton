#pragma once

#include "cm/Handles.hpp"

namespace tr {

// Maximum size of the ObjectData buffer array
constexpr auto MAX_OBJECTS = 128;
constexpr auto ALIGNMENT = 16;

struct alignas(ALIGNMENT) ObjectData {
   glm::mat4 model{};
   TextureHandle textureId{};
   uint32_t animationDataIndex{};
};

struct CameraData {
   glm::mat4 view{};
   glm::mat4 proj{};
   glm::mat4 viewProj{};
   glm::vec4 position{};
};

struct PushConstants {
   glm::vec4 lightPosition;
   glm::vec4 params;
};

struct GpuAnimationData {
   glm::mat4 jointMatrices;
};
}

