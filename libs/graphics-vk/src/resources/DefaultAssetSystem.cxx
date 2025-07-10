#include <algorithm>

#include "DefaultAssetSystem.hpp"
#include "api/fx/IAssetService.hpp"
#include "api/fx/IEventQueue.hpp"
#include "as/Model.hpp"
#include "bk/ThreadName.hpp"
#include "buffers/BufferSystem.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "img/ImageManager.hpp"
#include "img/TextureArena.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "resources/ByteConverters.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/GeometryAllocator.hpp"

namespace tr {

DefaultAssetSystem::DefaultAssetSystem(
    std::shared_ptr<IEventQueue> newEventQueue,
    std::shared_ptr<IAssetService> newAssetService,
    std::shared_ptr<BufferSystem> newBufferSystem,
    std::shared_ptr<GeometryBufferPack> newGeometryBufferPack,
    std::shared_ptr<TransferSystem> newTransferSystem,
    std::shared_ptr<GeometryAllocator> newGeometryAllocator,
    std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
    std::shared_ptr<TextureHandleMapper> newTextureHandleMapper,
    std::shared_ptr<ImageManager> newImageManager,
    std::shared_ptr<TextureArena> newTextureArena)
    : eventQueue{std::move(newEventQueue)},
      assetService{std::move(newAssetService)},
      bufferSystem{std::move(newBufferSystem)},
      geometryBufferPack{std::move(newGeometryBufferPack)},
      transferSystem{std::move(newTransferSystem)},
      geometryAllocator{std::move(newGeometryAllocator)},
      geometryHandleMapper{std::move(newGeometryHandleMapper)},
      textureHandleMapper{std::move(newTextureHandleMapper)},
      imageManager{std::move(newImageManager)},
      textureArena{std::move(newTextureArena)} {
  Log.trace("Constructing DefaultAssetSystem");
}

DefaultAssetSystem::~DefaultAssetSystem() {
  Log.trace("Destroying DefaultAssetSystem");
}

auto DefaultAssetSystem::run() -> void {
  ZoneScopedN("DefaultAssetSystem::run");
  Log.trace("DefaultAssetSystem::run()");

  thread = std::jthread([&](std::stop_token token) mutable {
    setCurrentThreadName("Assets");
    Log.trace("Started AssetSystemThread");
    // Create all subscriptions on the thread
    eventQueue->subscribe<BeginResourceBatch>(
        [this](const auto& batch) {
          eventBatches[batch->batchId] = std::vector<std::shared_ptr<EventVariant>>{};
        },
        "test_group");

    eventQueue->subscribe<StaticModelRequest>(
        [this](const StaticModelRequest& smRequest,
               const std::shared_ptr<EventVariant>& eventVariant) {
          eventBatches[smRequest.batchId].push_back(eventVariant);
        },
        "test_group");
    eventQueue->subscribe<StaticMeshRequest>(
        [this](const StaticMeshRequest& smRequest,
               const std::shared_ptr<EventVariant>& eventVariant) {
          eventBatches[smRequest.batchId].push_back(eventVariant);
        },
        "test_group");

    eventQueue->subscribe<DynamicModelRequest>(
        [this](const DynamicModelRequest& dmRequest,
               const std::shared_ptr<EventVariant>& eventVariant) {
          eventBatches[dmRequest.batchId].push_back(eventVariant);
        },
        "test_group");

    eventQueue->subscribe<EndResourceBatch>(
        [this](const std::shared_ptr<EndResourceBatch>& batch) {
          handleEndResourceBatch(batch->batchId);
        },
        "test_group");

    while (!token.stop_requested()) {
      eventQueue->dispatchPending();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    Log.trace("AssetSystem thread shutting down");
  });
}

auto DefaultAssetSystem::requestStop() -> void {
  thread.request_stop();
}

auto DefaultAssetSystem::handleEndResourceBatch(uint64_t batchId) -> void {
  ZoneScoped;
  auto uploadPlan = UploadPlan{.stagingBuffer = transferSystem->getTransferContext().stagingBuffer};
  auto imageUploadPlan =
      ImageUploadPlan{.stagingBuffer = transferSystem->getTransferContext().imageStagingBuffer};

  std::vector<StaticModelUploaded> responses{};
  auto i = 0;
  for (auto& eventVariant : eventBatches[batchId]) {
    if (eventVariant == nullptr) {
      Log.warn("eventBatch {} has a null event", i);
      continue;
    }
    auto visitor = [&](auto&& arg) -> void {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, StaticModelRequest>) {
        handleStaticModelRequest(arg, uploadPlan, imageUploadPlan, responses);
      }
      if constexpr (std::is_same_v<T, StaticMeshRequest>) {
        handleStaticMeshRequest(arg, uploadPlan, responses);
      }
      if constexpr (std::is_same_v<T, DynamicModelRequest>) {
        Log.trace("Handling Dynamic Model Request ID: {}", arg.requestId);
      }
    };
    std::visit(visitor, *eventVariant);
    ++i;
  }

  transferSystem->upload(uploadPlan, imageUploadPlan);

  for (const auto& upload : imageUploadPlan.uploads) {
    const auto& image = imageManager->getImage(upload.dstImage);
    const auto& sampler = imageManager->getSampler(imageManager->getDefaultSampler());
    auto handle = textureArena->insert(image.getImageView(), sampler);
    auto textureHandle = textureHandleMapper->toPublic(handle);
    for (auto& response : responses) {
      if (response.requestId == upload.requestId) {
        response.textureHandle.emplace(textureHandle);
      }
    }
  }

  for (const auto& response : responses) {
    eventQueue->emit(response);
  }
}

auto DefaultAssetSystem::handleStaticModelRequest(const StaticModelRequest& smRequest,
                                                  UploadPlan& uploadPlan,
                                                  ImageUploadPlan& imageUploadPlan,
                                                  std::vector<StaticModelUploaded>& responses)
    -> void {
  ZoneScoped;
  Log.trace("Handling Static Model Request ID: {}", smRequest.requestId);
  const auto& model = assetService->loadModel(smRequest.modelFilename);

  const auto geometryData = deInterleave(*model.staticVertices, model.indices);

  const auto [regionHandle, uploads] =
      geometryAllocator->allocate(*geometryData, transferSystem->getTransferContext());

  responses.push_back(StaticModelUploaded{
      .batchId = smRequest.batchId,
      .requestId = smRequest.requestId,
      .entityName = smRequest.entityName,
      .geometryHandle = geometryHandleMapper->toPublic(regionHandle),
  });

  uploadPlan.uploads.insert(uploadPlan.uploads.end(), uploads.begin(), uploads.end());

  const auto imageUploadData = fromImageData(model.imageData, smRequest.requestId);
  imageUploadPlan.uploads.insert(imageUploadPlan.uploads.end(),
                                 imageUploadData.begin(),
                                 imageUploadData.end());
}

auto DefaultAssetSystem::handleStaticMeshRequest(const StaticMeshRequest& smRequest,
                                                 UploadPlan& uploadPlan,
                                                 std::vector<StaticModelUploaded>& responses)
    -> void {
  ZoneScoped;
  Log.trace("Handling Static Mesh Request ID: {}", smRequest.requestId);
  const auto [regionHandle, uploads] =
      geometryAllocator->allocate(smRequest.geometryData, transferSystem->getTransferContext());

  responses.push_back(StaticModelUploaded{
      .batchId = smRequest.batchId,
      .requestId = smRequest.requestId,
      .entityName = smRequest.entityName,
      .geometryHandle = geometryHandleMapper->toPublic(regionHandle),
  });

  uploadPlan.uploads.insert(uploadPlan.uploads.end(), uploads.begin(), uploads.end());
}

auto DefaultAssetSystem::deInterleave(const std::vector<as::StaticVertex>& vertices,
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

  // TexCoord offsets i think are being set in bytes not elements

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

auto DefaultAssetSystem::fromImageData(const as::ImageData& imageData, uint64_t requestId)
    -> std::vector<ImageUploadData> {
  auto uploadDataList = std::vector<ImageUploadData>{};

  auto data = std::vector<std::byte>{};
  data.resize(imageData.data.size());
  std::ranges::transform(imageData.data, data.begin(), [](unsigned char c) {
    return static_cast<std::byte>(c);
  });

  const auto imageHandle = imageManager->createImage({
      .logicalName = "ModelTexture",
      .format = getVkFormat(imageData.bits, imageData.component),
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

auto DefaultAssetSystem::getVkFormat(int bits, int component) -> vk::Format {
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
