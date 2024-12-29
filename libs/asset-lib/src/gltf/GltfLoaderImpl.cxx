#include "as/gltf/GltfLoaderImpl.hpp"

namespace tr::as::gltf {

GltfLoaderImpl::~GltfLoaderImpl() noexcept {
}

bool GltfLoaderImpl::loadFromFile(tinygltf::Model* model,
                                  std::string* err,
                                  std::string* warn,
                                  const std::string& filename) {
  tinygltf::TinyGLTF loader{};
  return loader.LoadASCIIFromFile(model, err, warn, filename);
}
}
