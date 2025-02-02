#include "as/gltf/GltfGeometryExtractor.hpp"

#include "as/Model.hpp"

namespace as {

GltfGeometryExtractor::~GltfGeometryExtractor() noexcept = default;

void GltfGeometryExtractor::execute(const tinygltf::Model& model,
                                    const tinygltf::Primitive& primitive,
                                    const glm::mat4& transform,
                                    Model& tritonModel) {
  // Load Indices
  std::vector<uint32_t> indices;
  {
    const auto& accessor = model.accessors[primitive.indices];
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];

    // glTF supports different component types of indices
    switch (accessor.componentType) {
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
        const auto* buf = reinterpret_cast<const uint32_t*>(
            &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for (size_t index = 0; index < accessor.count; index++) {
          indices.push_back(buf[index]);
        }
        break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
        const auto* buf = reinterpret_cast<const uint16_t*>(
            &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for (size_t index = 0; index < accessor.count; index++) {
          indices.push_back(buf[index]);
        }
        break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
        const auto* buf = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
        for (size_t index = 0; index < accessor.count; index++) {
          indices.push_back(buf[index]);
        }
        break;
      }
      default:
        std::stringstream err;
        err << "Index component type " << accessor.componentType;
        throw std::runtime_error(err.str());
    }
    tritonModel.indices = std::move(indices);
  }

  // Static Model
  if (model.skins.empty()) {
    std::vector<StaticVertex> vertices;
    for (const auto& [attribute, value] : primitive.attributes) {
      const auto& accessor = model.accessors[value];

      const auto& vertexCount = accessor.count;
      const auto& view = model.bufferViews[accessor.bufferView];
      const auto dataOffset = accessor.byteOffset + view.byteOffset;
      const auto& buffer = model.buffers[view.buffer];

      // NOLINTNEXTLINE
      const auto& data = reinterpret_cast<const float*>(&buffer.data[dataOffset]);

      vertices.resize(vertexCount);

      if (attribute == "POSITION") {
        for (size_t i = 0; i < vertexCount; i++) {
          auto vertexPosition = glm::make_vec3(&data[i * 3]);
          auto tempVec = transform * glm::vec4(vertexPosition, 1.f);
          vertices[i].position = glm::vec3(tempVec);
        }
      }
      if (attribute == "TEXCOORD_0") {
        for (size_t i = 0; i < vertexCount; i++) {
          vertices[i].texCoord = glm::make_vec2(&data[i * 2]);
        }
      }
    }
    tritonModel.staticVertices = std::move(vertices);
    tritonModel.vertexType = VertexType::Static;
  } else {
    // Skinned Model
    std::vector<DynamicVertex> vertices;
    for (const auto& [attribute, value] : primitive.attributes) {
      const auto& accessor = model.accessors[value];

      const auto& vertexCount = accessor.count;
      const auto& view = model.bufferViews[accessor.bufferView];
      const auto dataOffset = accessor.byteOffset + view.byteOffset;
      const auto& buffer = model.buffers[view.buffer];

      // NOLINTNEXTLINE
      const auto& data = reinterpret_cast<const float*>(&buffer.data[dataOffset]);

      vertices.resize(vertexCount);

      if (attribute == "POSITION") {
        for (size_t i = 0; i < vertexCount; i++) {
          auto vertexPosition = glm::make_vec3(&data[i * 3]);
          auto tempVec = transform * glm::vec4(vertexPosition, 1.f);
          vertices[i].position = glm::vec3(tempVec);
        }
      }
      if (attribute == "TEXCOORD_0") {
        for (size_t i = 0; i < vertexCount; i++) {
          vertices[i].texCoord = glm::make_vec2(&data[i * 2]);
        }
      }
      if (attribute == "JOINTS_0") {
        for (size_t i = 0; i < vertexCount; i++) {
          const auto& jointData = reinterpret_cast<const uint8_t*>(&buffer.data[dataOffset]);
          vertices[i].joint0 = glm::make_vec4(&jointData[i * 4]);
        }
      }
      if (attribute == "WEIGHTS_0") {
        for (size_t i = 0; i < vertexCount; i++) {
          vertices[i].weight0 = glm::make_vec4(&data[i * 4]);
        }
      }
    }
    tritonModel.dynamicVertices = std::move(vertices);
    tritonModel.vertexType = VertexType::Dynamic;
  }
}
}
