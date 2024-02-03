#pragma once

namespace Triton {

   struct RenderObject {
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