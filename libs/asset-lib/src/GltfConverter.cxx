#include "GltfConverter.hpp"

namespace al::gltf {
   Converter::~Converter() noexcept {
   }

   auto Converter::convert(const std::filesystem::path& gltf, const std::filesystem::path& skeleton)
       -> Model {

      tinygltf::Model model;
      tinygltf::TinyGLTF loader;
      std::string err;
      std::string warn;

      bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, gltf.string());
      if (!warn.empty()) {
         throw std::runtime_error(warn);
      }

      if (!err.empty()) {
         throw std::runtime_error(err);
      }

      if (!ret) {
         Log::error << "Failed to parse glTF file" << std::endl;
         throw std::runtime_error("Failed to parse glTF file");
      }

      auto loadedTextureIndices = std::unordered_map<int, ImageHandle>{};

      auto texturedGeometryHandle = TexturedGeometryHandle{};

      const auto& scene = model.scenes[model.defaultScene];
      for (const auto& nodeIndex : scene.nodes) {
         parseNode(model, model.nodes[nodeIndex], loadedTextureIndices, texturedGeometryHandle);
      }

      return Model{};
   }
}
