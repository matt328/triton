#include "as/gltf/ModelLoader.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

namespace tr::as::gltf {

ModelLoader::~ModelLoader() noexcept {
}

auto ModelLoader::load(GltfFileLoader* loader, const std::filesystem::path& path) const
    -> tinygltf::Model {

  tinygltf::Model model;
  std::string err;
  std::string warn;

  const bool ret = loader->loadFromFile(&model, &err, &warn, path.string());

  if (!warn.empty()) {
    throw std::runtime_error(warn);
  }

  if (!err.empty()) {
    throw std::runtime_error(err);
  }

  if (!ret) {
    Log.error("Failed to parse glTF file");
    throw std::runtime_error("Failed to parse glTF file");
  }

  return model;
}

}
