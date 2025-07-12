#include "ModelProcessor.hpp"
#include "api/fx/IAssetService.hpp"
#include "img/ImageManager.hpp"
#include "resources/ByteConverters.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/GeometryAllocator.hpp"

namespace tr {

auto ModelProcessor::handle(const std::shared_ptr<StaticModelRequest>& request,
                            UploadPlan& uploadPlan,
                            ImageUploadPlan& imagePlan,
                            InFlightUploadMap& inFlightUploads,
                            const AssetSystems& assetSystems) -> void {
  ZoneScoped;
  Log.trace("Handling Static Model Request ID: {}", request->requestId);

  const auto& model = assetSystems.assetService->loadModel(request->modelFilename);
  const auto geometryData = deInterleave(*model.staticVertices, model.indices);
  const auto [regionHandle, uploads] =
      assetSystems.geometryAllocator->allocate(*geometryData,
                                               assetSystems.transferSystem->getTransferContext());

  inFlightUploads.emplace(request->requestId, InFlightUpload::from(*request));

  for (const auto& upload : uploads) {
    uploadPlan.uploadsByRequest[request->requestId].push_back(upload);
  }
  uploadPlan.geometryDataByRequest.emplace(
      request->requestId,
      assetSystems.geometryHandleMapper->toPublic(regionHandle));

  const auto imageUploadData = fromImageData(model.imageData,
                                             request->requestId,
                                             assetSystems.imageManager,
                                             assetSystems.transferSystem);
  for (const auto& imageUpload : imageUploadData) {
    imagePlan.uploadsByRequest[request->requestId].push_back(imageUpload);
  }
}

/// Eventually Update the TRM model formats to store data on disk in a deinterleaved format so
/// this method is unnecessary, but just convert it here for now.
auto ModelProcessor::deInterleave(const std::vector<as::StaticVertex>& vertices,
                                  const std::vector<uint32_t>& indexData)
    -> std::unique_ptr<GeometryData> {
  auto positions = std::make_shared<std::vector<GpuVertexPositionData>>();
  auto texCoords = std::make_shared<std::vector<GpuVertexTexCoordData>>();
  auto colors = std::make_shared<std::vector<GpuVertexColorData>>();
  auto indices = std::make_shared<std::vector<GpuIndexData>>();

  positions->reserve(vertices.size() * sizeof(GpuVertexPositionData));
  texCoords->reserve(vertices.size() * sizeof(GpuVertexTexCoordData));
  colors->reserve(vertices.size() * sizeof(GpuVertexColorData));
  indices->reserve(indexData.size() * sizeof(GpuIndexData));

  for (const auto& vertex : vertices) {
    positions->emplace_back(vertex.position);
    texCoords->emplace_back(vertex.texCoord);
  }

  for (auto index : indexData) {
    indices->emplace_back(index);
  }

  auto indicesBytes = toByteVector(indices);
  auto texCoordBytes = toByteVector(texCoords);
  auto positionBytes = toByteVector(positions);

  return std::make_unique<GeometryData>(GeometryData{.indexData = indicesBytes,
                                                     .positionData = positionBytes,
                                                     .colorData = nullptr,
                                                     .texCoordData = texCoordBytes,
                                                     .normalData = nullptr,
                                                     .animationData = nullptr});
}

auto ModelProcessor::fromImageData(const as::ImageData& imageData,
                                   uint64_t requestId,
                                   const std::shared_ptr<ImageManager>& imageManager,
                                   const std::shared_ptr<TransferSystem>& transferSystem)
    -> std::vector<ImageUploadData> {
  auto uploadDataList = std::vector<ImageUploadData>{};

  auto data = std::vector<std::byte>{};
  data.resize(imageData.data.size());
  std::ranges::transform(imageData.data, data.begin(), [](unsigned char c) {
    return static_cast<std::byte>(c);
  });

  const auto imageHandle = imageManager->createImage({
      .logicalName = "ModelTexture",
      .format = ModelProcessor::getVkFormat(imageData.bits, imageData.component),
      .extent =
          vk::Extent2D{
              .width = static_cast<uint32_t>(imageData.width),
              .height = static_cast<uint32_t>(imageData.height),
          },
      .usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .debugName = "ModelTexture",
  });

  auto stagingBufferOffset =
      transferSystem->getTransferContext().imageStagingAllocator->allocate({.size = data.size()});

  uploadDataList.push_back(ImageUploadData{
      .data = std::make_shared<std::vector<std::byte>>(data),
      .dataSize = data.size(),
      .dstImage = imageHandle,
      .subresource = vk::ImageSubresourceLayers{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                .mipLevel = 0,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1},
      .imageExtent = vk::Extent3D{.width = static_cast<uint32_t>(imageData.width),
                                  .height = static_cast<uint32_t>(imageData.height),
                                  .depth = 1},
      .stagingBufferOffset = stagingBufferOffset->offset,
      .requestId = requestId});

  return uploadDataList;
}

auto ModelProcessor::getVkFormat(int bits, int component) -> vk::Format {
  if (bits == 8) {
    switch (component) {
      case 1:
        return vk::Format::eR8Unorm;
      case 2:
        return vk::Format::eR8G8Unorm;
      case 3:
        return vk::Format::eR8G8B8Unorm;
      case 4:
        return vk::Format::eR8G8B8A8Unorm;
    }
  } else if (bits == 16) {
    switch (component) {
      case 1:
        return vk::Format::eR16Unorm;
      case 2:
        return vk::Format::eR16G16Unorm;
      case 3:
        return vk::Format::eR16G16B16Unorm;
      case 4:
        return vk::Format::eR16G16B16A16Unorm;
    }
  } else if (bits == 32) {
    switch (component) {
      case 1:
        return vk::Format::eR32Sfloat;
      case 2:
        return vk::Format::eR32G32Sfloat;
      case 3:
        return vk::Format::eR32G32B32Sfloat;
      case 4:
        return vk::Format::eR32G32B32A32Sfloat;
    }
  }

  throw std::runtime_error("Unsupported image format: component=" + std::to_string(component) +
                           ", bits=" + std::to_string(bits));
}

}
