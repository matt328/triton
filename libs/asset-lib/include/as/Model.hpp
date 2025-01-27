#pragma once

#include "Vertex.hpp"
#include "BaseException.hpp"
#include "as/StaticVertex.hpp"
#include "as/VertexTypes.hpp"

namespace as {

constexpr uint32_t SERIAL_VERSION = 2;

class SerializationException final : public tr::BaseException {
  using BaseException::BaseException;
};

class ImageData {
public:
  std::vector<unsigned char> data{};
  int width{};
  int height{};
  int component{};
  template <class Archive>
  void serialize(Archive& archive) {
    archive(data, width, height, component);
  }
};

struct Model {
  VertexType vertexType;
  std::optional<std::vector<as::Vertex>> skinnedVertices;
  std::optional<std::vector<as::StaticVertex>> staticVertices;
  std::vector<uint32_t> indices;
  std::unordered_map<int, int> jointRemaps;
  std::vector<glm::mat4> inverseBindPoses;
  ImageData imageData;

  [[nodiscard]] auto skinned() const {
    return !inverseBindPoses.empty();
  }

  [[nodiscard]] auto numJoints() const {
    return static_cast<int>(inverseBindPoses.size());
  }

  template <class Archive>
  void serialize(Archive& archive, std::uint32_t const version) {
    if (version != 1) {
      auto msg = fmt::format("Version mismatch in as:Model, found {0}, expected {1}",
                             version,
                             as::SERIAL_VERSION);
      throw SerializationException(msg);
    }
    archive(vertexType);

    if (vertexType == VertexType::Skinned) {
      archive(skinnedVertices, jointRemaps, inverseBindPoses);
    } else if (vertexType == VertexType::Static) {
      archive(staticVertices);
    }

    archive(indices, imageData);
  }
};
}

CEREAL_CLASS_VERSION(as::Model, as::SERIAL_VERSION);
