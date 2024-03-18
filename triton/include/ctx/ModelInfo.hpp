#pragma once

namespace tr::util {
   class KtxImage;
}

namespace tr::ctx {

   // TODO: std::move in the initializer list is probably redundant here, profile memory to confirm
   class ModelInfo {
    public:
      ModelInfo(fastgltf::Asset&& asset, const std::filesystem::path& filePath);
      ~ModelInfo();

      // Move but don't copy
      ModelInfo(ModelInfo&&) = default;
      ModelInfo& operator=(ModelInfo&&) = default;

      ModelInfo(const ModelInfo&) = delete;
      ModelInfo& operator=(const ModelInfo&) = delete;

    private:
      fastgltf::Asset asset;
      std::vector<util::KtxImage> loadedImages;
   };
}
