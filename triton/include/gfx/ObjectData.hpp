#pragma once

#include "Handles.hpp"

namespace tr::gfx {

   // Maximum size of the ObjectData buffer array
   constexpr auto MAX_OBJECTS = 1000;

   struct alignas(16) ObjectData {
      glm::mat4 model;
      TextureHandle textureId;
   };

   /// This struct is copyable so make sure you know what you're doing.
   struct CameraData {
      CameraData(glm::mat4 view, glm::mat4 proj, glm::mat4 viewProj)
          : view(view), proj(proj), viewProj(viewProj) {
      }

      CameraData() = default;

      ~CameraData() = default;

      CameraData(CameraData&& other) noexcept = default;
      CameraData& operator=(CameraData&&) = default;

      CameraData(const CameraData&) = default;
      CameraData& operator=(const CameraData&) = default;

      glm::mat4 view{};
      glm::mat4 proj{};
      glm::mat4 viewProj{};
   };
}