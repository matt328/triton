#pragma once

#include "Handles.hpp"

namespace Triton::Graphics {

   // Maximum size of the ObjectData buffer array
   constexpr auto MAX_OBJECTS = 1000;

   struct ObjectData {
      glm::mat4 model;
      TextureHandle textureId;
   };

   struct CameraData {
      glm::mat4 view;
      glm::mat4 proj;
      glm::mat4 viewProj;
   };
}