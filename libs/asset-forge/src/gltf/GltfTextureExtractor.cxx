#include "as/gltf/GltfTextureExtractor.hpp"

#include "as/Model.hpp"

namespace as {

GltfTextureExtractor::~GltfTextureExtractor() noexcept {
}

void GltfTextureExtractor::execute(const tinygltf::Model& model,
                                   const int textureIndex,
                                   Model& tritonModel) {
  if (textureIndex == -1) {
    tinygltf::Image image;

    image.width = 1;
    image.height = 1;
    image.component = 4;
    image.bits = 8;
    image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

    std::vector<unsigned char> imageData(image.width * image.height * image.component);
    imageData[0] = 255; // Red
    imageData[1] = 255; // Green
    imageData[2] = 255; // Blue
    imageData[3] = 255; // Alpha

    image.image = std::move(imageData);
    tritonModel.imageData = ImageData{.data = image.image,
                                      .width = image.width,
                                      .height = image.height,
                                      .component = image.component};
  } else {
    const auto& texture = model.textures[textureIndex];
    const auto& image = model.images[texture.source];
    tritonModel.imageData = ImageData{.data = image.image,
                                      .width = image.width,
                                      .height = image.height,
                                      .component = image.component};
  }
}
}
