#pragma once

#include "cm/Handles.hpp"

namespace tr::gfx {

   struct RenderObject {
      RenderObject(RenderObject&& other) noexcept = default;

      RenderObject(cm::MeshHandle meshId, cm::TextureHandle textureId, glm::mat4 modelMatrix)
          : meshId(meshId), textureId(textureId), modelMatrix(modelMatrix) {
      }

      // Avoid accidentally copying this because lots of these are moved around during the rendering
      // loop so always std::move() this when passing it around.
      RenderObject(const RenderObject&) = delete;
      RenderObject& operator=(const RenderObject&) = delete;
      RenderObject& operator=(RenderObject&&) = delete;
      ~RenderObject() = default;

      cm::MeshHandle meshId;
      cm::TextureHandle textureId;
      glm::mat4 modelMatrix;
   };

   struct PerFrameData {
      glm::mat4 view;
      glm::mat4 proj;
      glm::mat4 viewProj;
   };
}