#pragma once

#include "BaseException.hpp"
#include "as/DynamicVertex.hpp"
#include "as/StaticVertex.hpp"
#include "as/VertexTypes.hpp"

namespace as {

constexpr uint32_t SERIAL_VERSION = 4;

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
  std::optional<std::vector<as::DynamicVertex>> dynamicVertices;
  std::optional<std::vector<as::StaticVertex>> staticVertices;
  std::vector<uint32_t> indices;
  std::map<int, int> jointRemaps;
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
    if (version != as::SERIAL_VERSION) {
      auto msg = fmt::format("Version mismatch in as:Model, found {0}, expected {1}",
                             version,
                             as::SERIAL_VERSION);
      throw SerializationException(msg);
    }
    archive(vertexType);

    if (vertexType == VertexType::Dynamic) {
      archive(dynamicVertices, jointRemaps, inverseBindPoses);
    } else if (vertexType == VertexType::Static) {
      archive(staticVertices);
    }

    archive(indices, imageData);
  }
};
}

CEREAL_CLASS_VERSION(as::Model, as::SERIAL_VERSION);
