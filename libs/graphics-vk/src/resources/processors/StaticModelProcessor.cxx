#include "StaticModelProcessor.hpp"
#include "api/fx/IAssetService.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/GeometryAllocator.hpp"
#include "resources/processors/Helpers.hpp"

namespace tr {

StaticModelProcessor::StaticModelProcessor(
    std::shared_ptr<IAssetService> newAssetService,
    std::shared_ptr<GeometryAllocator> newGeometryAllocator,
    std::shared_ptr<TransferSystem> newTransferSystem,
    std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
    std::shared_ptr<ImageManager> newImageManager)
    : ImageProcessor{std::move(newImageManager), newTransferSystem},
      assetService{std::move(newAssetService)},
      geometryAllocator{std::move(newGeometryAllocator)},
      transferSystem{newTransferSystem},
      geometryHandleMapper{std::move(newGeometryHandleMapper)} {
}

auto StaticModelProcessor::handles(std::type_index typeIndex) const -> bool {
  return thisType == typeIndex;
}

auto StaticModelProcessor::analyze(uint64_t batchId, std::shared_ptr<void> request)
    -> StagingRequirements {
  auto smRequest = std::static_pointer_cast<StaticModelRequest>(request);
  const auto& model = assetService->loadModel(smRequest->modelFilename);
  const auto imageDataSize = analyzeImageData(model.imageData, smRequest->requestId);
  const auto geometryData = processorHelpers::deInterleave(*model.staticVertices, model.indices);
  return {
      .cargo =
          {
              .batchId = batchId,
              .requestId = smRequest->requestId,
              .entityName = smRequest->entityName,
          },
      .responseType = typeid(StaticModelUploaded),
      .geometrySize = geometryData->getSize(),
      .imageSize = imageDataSize.imageSize,
      .geometryData = geometryData,
      .imageDataList = {std::make_shared<as::ImageData>(model.imageData)},
  };
}

auto StaticModelProcessor::process(std::shared_ptr<void> request) -> ProcessingResult {
  ZoneScoped;
  auto smRequest = std::static_pointer_cast<StaticModelRequest>(request);
  Log.trace("Handling Static Model Request ID: {}", smRequest->requestId);

  auto result = ProcessingResult{};
  result.requestId = smRequest->requestId;

  const auto& model = assetService->loadModel(smRequest->modelFilename);
  const auto geometryData = processorHelpers::deInterleave(*model.staticVertices, model.indices);
  auto regionHandle = Handle<GeometryRegion>{};
  // Maybe geometry allocator should just have the transferSystem injected into it?
  std::tie(regionHandle, result.geometryUploads) =
      geometryAllocator->allocate(*geometryData, transferSystem->getTransferContext());

  const auto geometryHandle = geometryHandleMapper->toPublic(regionHandle);

  result.responseEvent = StaticModelUploaded{.batchId = smRequest->batchId,
                                             .requestId = smRequest->requestId,
                                             .entityName = smRequest->entityName,
                                             .geometryHandle = geometryHandle};
  result.geometryHandle = geometryHandle;

  result.imageUploads.push_back(processImageData(model.imageData, smRequest->requestId));

  return result;
}

}
