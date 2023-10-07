#pragma once

#include <glm/glm.hpp>

struct RenderObject {
   std::string meshId;
   uint32_t textureId;
   glm::mat4 modelMatrix;
};
