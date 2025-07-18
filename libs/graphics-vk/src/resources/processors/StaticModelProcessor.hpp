#pragma once

#include "api/fx/ResourceEvents.hpp"
#include "gfx/HandleMapperTypes.hpp"
#include "img/ImageManager.hpp"
#include "ImageProcessor.hpp"

namespace tr {

class IAssetService;
class GeometryAllocator;
class TransferSystem;

class StaticModelProcessor : public ImageProcessor {
public:
  StaticModelProcessor(std::shared_ptr<IAssetService> newAssetService,
                       std::shared_ptr<TransferSystem> newTransferSystem,
                       std::shared_ptr<ImageManager> newImageManager);
  ~StaticModelProcessor() override = default;

  StaticModelProcessor(const StaticModelProcessor&) = delete;
  StaticModelProcessor(StaticModelProcessor&&) = delete;
  auto operator=(const StaticModelProcessor&) -> StaticModelProcessor& = delete;
  auto operator=(StaticModelProcessor&&) -> StaticModelProcessor& = delete;

  [[nodiscard]] auto handles(std::type_index typeIndex) const -> bool override;
  auto analyze(uint64_t batchId, std::shared_ptr<void> request) -> StagingRequirements override;

private:
  std::shared_ptr<IAssetService> assetService;

  std::type_index thisType = typeid(StaticModelRequest);
};

}
