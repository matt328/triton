#pragma once

#include "graphics/Handles.hpp"

namespace Triton::Graphics {

   struct RenderObject {
      RenderObject(RenderObject&& other) noexcept = default;

      RenderObject(MeshHandle meshId, uint32_t textureId, glm::mat4 modelMatrix)
          : meshId(meshId), textureId(textureId), modelMatrix(modelMatrix) {
      }

      // Avoid accidentally copying this because lots of these are moved around during the rendering
      // loop so always std::move() this when passing it around.
      RenderObject(const RenderObject&) = delete;
      RenderObject& operator=(const RenderObject&) = delete;
      RenderObject& operator=(RenderObject&&) = delete;
      ~RenderObject() = default;

      MeshHandle meshId;
      uint32_t textureId;
      glm::mat4 modelMatrix;
   };

   struct PerFrameData {
      glm::mat4 view;
      glm::mat4 proj;
      glm::mat4 viewProj;
   };
}