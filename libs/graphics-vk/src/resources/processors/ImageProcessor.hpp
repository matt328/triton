#pragma once

#include "IResourceProcessor.hpp"

namespace tr {

class ImageManager;
class TransferSystem;

class ImageProcessor : public IResourceProcessor {
public:
  ImageProcessor(std::shared_ptr<ImageManager> newImageManager,
                 std::shared_ptr<TransferSystem> newTransferSystem);
  ~ImageProcessor() override = default;

  ImageProcessor(const ImageProcessor&) = delete;
  ImageProcessor(ImageProcessor&&) = delete;
  auto operator=(const ImageProcessor&) -> ImageProcessor& = delete;
  auto operator=(ImageProcessor&&) -> ImageProcessor& = delete;

protected:
  static auto getVkFormat(int bits, int component) -> vk::Format;
  static auto analyzeImageData(const as::ImageData& imageData,
                               const Cargo& cargo,
                               std::type_index responseType) -> StagingRequirements;

private:
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<TransferSystem> transferSystem;
};

}
