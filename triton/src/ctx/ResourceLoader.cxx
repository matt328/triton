#include "ctx/ResourceLoader.hpp"

#include "ctx/ModelInfo.hpp"
#include "ctx/KtxImage.hpp"
#include "gfx/Handles.hpp"

namespace tr::ctx {

   using gfx::ModelInfoHandle;

   ResourceLoader::ResourceLoader(gfx::Renderer& renderer) : renderer(renderer) {
   }

   ResourceLoader::~ResourceLoader() {
   }

   std::future<ModelInfoHandle> ResourceLoader::loadGltf(std::filesystem::path& path) {
      return executor.async("Load gltf", [path, this]() { return loadGltfInt(path); });
   }

   std::future<gfx::MeshMaterialHandle> ResourceLoader::loadGltfAndUpload(
       std::filesystem::path& path) {
      Log::info << "loadGltfAndUpload with path: " << path << std::endl;
      return executor.async("Load and Upload gltf",
                            [path, this]() { return uploadModelInfoInt(loadGltfInt(path)); });
   }

   std::future<gfx::MeshMaterialHandle> ResourceLoader::uploadRenderObjectData(
       const gfx::ModelInfoHandle handle) {
      Log::info << "uploadRenderObjectData handle: " << handle << std::endl;
      return std::async(std::launch::deferred,
                        []() { return static_cast<gfx::MeshMaterialHandle>(3); });
   }

   gfx::MeshMaterialHandle ResourceLoader::uploadModelInfoInt(
       const gfx::ModelInfoHandle modelHandle) {
      Log::info << "uploadModelInfoInt handle: " << modelHandle << std::endl;
      return static_cast<gfx::MeshMaterialHandle>(2);
   }

   ModelInfoHandle ResourceLoader::loadGltfInt(const std::filesystem::path& path) {
      ZoneNamedN(loadGltf, "Loading Model", true);
      fastgltf::Parser parser{};
      fastgltf::GltfDataBuffer data{};

      data.loadFromFile(path);

      auto asset =
          parser.loadGltf(&data, path.parent_path(), fastgltf::Options::LoadExternalBuffers);
      if (auto error = asset.error(); error != fastgltf::Error::None) {
         std::stringstream ss{};
         ss << "Failed to load gltf file: " << path.string() << fastgltf::to_underlying(error)
            << std::endl;
         throw std::runtime_error(ss.str());
      }

      const auto position = modelInfoList.size();

      modelInfoList.emplace_back(std::move(asset.get()), path);

      return static_cast<ModelInfoHandle>(position);
   }
}
