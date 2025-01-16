#include "as/gltf/GltfLoaderImpl.hpp"

namespace as {

GltfLoaderImpl::~GltfLoaderImpl() noexcept {
}

auto GltfLoaderImpl::loadFromFile(tinygltf::Model* model,
                                  std::string* err,
                                  std::string* warn,
                                  const std::string& filename) -> bool {
  tinygltf::TinyGLTF loader{};
  return loader.LoadASCIIFromFile(model, err, warn, filename);
}
}
