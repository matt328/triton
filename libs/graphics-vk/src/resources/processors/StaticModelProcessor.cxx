#include "StaticModelProcessor.hpp"
#include "api/fx/IAssetService.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/processors/Helpers.hpp"

namespace tr {

StaticModelProcessor::StaticModelProcessor(std::shared_ptr<IAssetService> newAssetService,
                                           std::shared_ptr<TransferSystem> newTransferSystem,
                                           std::shared_ptr<ImageManager> newImageManager)
    : ImageProcessor{std::move(newImageManager), newTransferSystem},
      assetService{std::move(newAssetService)} {
}

auto StaticModelProcessor::handles(std::type_index typeIndex) const -> bool {
  return thisType == typeIndex;
}

auto StaticModelProcessor::analyze(uint64_t batchId, std::shared_ptr<void> request)
    -> StagingRequirements {
  auto smRequest = std::static_pointer_cast<StaticModelRequest>(request);
  const auto& model = assetService->loadModel(smRequest->modelFilename);
  const auto cargo = Cargo{
      .batchId = batchId,
      .requestId = smRequest->requestId,
      .entityName = smRequest->entityName,
  };
  const auto imageDataSize = analyzeImageData(model.imageData, cargo, typeid(StaticModelUploaded));
  const auto geometryData = processorHelpers::deInterleave(*model.staticVertices, model.indices);
  return {
      .cargo = cargo,
      .responseType = typeid(StaticModelUploaded),
      .geometrySize = geometryData->getSize(),
      .imageSize = imageDataSize.imageSize,
      .geometryData = geometryData,
      .imageDataList = {std::make_shared<as::ImageData>(model.imageData)},
  };
}

}
