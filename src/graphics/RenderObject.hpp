#pragma once

namespace Triton::Graphics {

   struct RenderObject {
      RenderObject(RenderObject&& other) noexcept
          : meshId(other.meshId), textureId(other.textureId), modelMatrix(other.modelMatrix) {
      }

      RenderObject(std::string meshId, uint32_t textureId, glm::mat4 modelMatrix)
          : meshId(std::move(meshId)), textureId(textureId), modelMatrix(modelMatrix) {
      }

      RenderObject(const RenderObject&) = delete;
      RenderObject& operator=(const RenderObject&) = delete;
      RenderObject& operator=(RenderObject&&) = delete;
      ~RenderObject() = default;

      std::string meshId;
      uint32_t textureId;
      glm::mat4 modelMatrix;
   };

   struct PerFrameData {
      glm::mat4 view;
      glm::mat4 proj;
      glm::mat4 viewProj;
   };
}