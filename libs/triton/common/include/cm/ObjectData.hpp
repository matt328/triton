#pragma once

#include "cm/Handles.hpp"

namespace tr::cm::gpu {

   // Maximum size of the ObjectData buffer array
   constexpr auto MAX_OBJECTS = 128;

   struct alignas(16) ObjectData {
      glm::mat4 model;
      cm::TextureHandle textureId;
      uint32_t animationDataIndex;
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

   struct AnimationData {
      glm::mat4 jointMatrices;
   };
}