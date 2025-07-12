#pragma once

#include "api/fx/ResourceEvents.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "InFlightUpload.hpp"
#include "gfx/HandleMapperTypes.hpp"

namespace tr {

class ImageManager;
class TransferSystem;
class IAssetService;
class GeometryAllocator;

struct AssetSystems {
  std::shared_ptr<IAssetService> assetService;
  std::shared_ptr<GeometryAllocator> geometryAllocator;
  std::shared_ptr<TransferSystem> transferSystem;
  std::shared_ptr<GeometryHandleMapper> geometryHandleMapper;
  std::shared_ptr<ImageManager> imageManager;
};

class ModelProcessor {
private:
  ModelProcessor() = default;
  ~ModelProcessor() = default;

public:
  ModelProcessor(const ModelProcessor&) = delete;
  ModelProcessor(ModelProcessor&&) = delete;
  auto operator=(const ModelProcessor&) -> ModelProcessor& = delete;
  auto operator=(ModelProcessor&&) -> ModelProcessor& = delete;

  static auto handle(const std::shared_ptr<StaticModelRequest>& request,
                     UploadPlan& uploadPlan,
                     ImageUploadPlan& imagePlan,
                     InFlightUploadMap& inFlightUploads,
                     const AssetSystems& assetSystems) -> void;

  /// Eventually Update the TRM model formats to store data on disk in a deinterleaved format so
  /// this method is unnecessary, but just convert it here for now.
  static auto deInterleave(const std::vector<as::StaticVertex>& vertices,
                           const std::vector<uint32_t>& indexData) -> std::unique_ptr<GeometryData>;

  static auto fromImageData(const as::ImageData& imageData,
                            uint64_t requestId,
                            const std::shared_ptr<ImageManager>& imageManager,
                            const std::shared_ptr<TransferSystem>& transferSystem)
      -> std::vector<ImageUploadData>;

  static auto getVkFormat(int bits, int component) -> vk::Format;
};

}
