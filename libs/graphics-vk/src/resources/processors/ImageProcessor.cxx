#include "ImageProcessor.hpp"
#include "img/ImageManager.hpp"
#include "resources/TransferSystem.hpp"

namespace tr {

ImageProcessor::ImageProcessor(std::shared_ptr<ImageManager> newImageManager,
                               std::shared_ptr<TransferSystem> newTransferSystem)
    : imageManager{std::move(newImageManager)}, transferSystem{std::move(newTransferSystem)} {
}

auto ImageProcessor::analyzeImageData(const as::ImageData& imageData,
                                      const Cargo& cargo,
                                      std::type_index responseType) -> StagingRequirements {

  return {.cargo = cargo,
          .responseType = responseType,
          .imageSize = imageData.data.size(),
          .geometryData = nullptr,
          .imageDataList = {std::make_shared<as::ImageData>(imageData)}};
}

}
