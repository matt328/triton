#include "as/gltf/TransformParser.hpp"

namespace tr::as::gltf {

TransformParser::~TransformParser() noexcept {
}

glm::mat4 TransformParser::execute(const tinygltf::Node& node) const {
  // If glft file has a matrix, we should prefer that
  if (node.matrix.size() == 16) {
    const auto floatVec = std::vector<float>{node.matrix.begin(), node.matrix.end()};
    return glm::make_mat4(floatVec.data());
  }
  // If none of these exist, it will end up with an identity matrix
  auto translation = glm::vec3{0.f, 0.f, 0.f};
  auto rotation = glm::identity<glm::quat>();
  auto scale = glm::vec3(1.f, 1.f, 1.f);

  if (node.translation.size() == 3) {
    translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
  }

  if (node.rotation.size() == 4) {
    rotation = glm::quat(static_cast<float>(node.rotation[3]),
                         static_cast<float>(node.rotation[0]),
                         static_cast<float>(node.rotation[1]),
                         static_cast<float>(node.rotation[2]));
  }

  if (node.scale.size() == 3) {
    scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
  }

  const auto translationMatrix = glm::translate(glm::mat4(1.f), translation);
  const auto rotationMatrix = glm::mat4_cast(rotation);
  const auto scaleMatrix = glm::scale(glm::mat4(1.f), scale);

  return translationMatrix * rotationMatrix * scaleMatrix;
}
}
