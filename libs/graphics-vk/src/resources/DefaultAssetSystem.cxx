#include "DefaultAssetSystem.hpp"
#include "api/fx/IAssetService.hpp"
#include "api/fx/IEventQueue.hpp"
#include "as/Model.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/BufferSystem.hpp"
#include "buffers/DeviceBufferSystem.hpp"
#include "buffers/UploadPlan.hpp"
#include "r3/GeometryBufferPack.hpp"

namespace tr {

DefaultAssetSystem::DefaultAssetSystem(std::shared_ptr<IEventQueue> newEventQueue,
                                       std::shared_ptr<IAssetService> newAssetService,
                                       std::shared_ptr<DeviceBufferSystem> newDeviceBufferSystem,
                                       std::shared_ptr<BufferSystem> newBufferSystem,
                                       std::shared_ptr<GeometryBufferPack> newGeometryBufferPack)
    : eventQueue{std::move(newEventQueue)},
      assetService{std::move(newAssetService)},
      deviceBufferSystem{std::move(newDeviceBufferSystem)},
      bufferSystem{std::move(newBufferSystem)},
      geometryBufferPack{std::move(newGeometryBufferPack)},
      stagingBufferHandle{
          bufferSystem->registerBuffer(BufferCreateInfo{.bufferType = BufferType::HostTransient,
                                                        .bufferUsage = BufferUsage::Storage,
                                                        .initialSize = 10240,
                                                        .debugName = "Buffer-GeometryStaging"})} {
  Log.trace("Constructing DefaultAssetSystem");

  eventQueue->subscribe<BeginResourceBatch>(
      [this](const BeginResourceBatch& batch) {
        eventBatches[batch.batchId] = std::vector<const EventVariant*>{};
      },
      "test_group");

  eventQueue->subscribe<StaticModelRequest>(
      [this](const StaticModelRequest& smRequest, const EventVariant& eventVariant) {
        eventBatches[smRequest.batchId].push_back(&eventVariant);
      },
      "test_group");

  eventQueue->subscribe<DynamicModelRequest>(
      [this](const DynamicModelRequest& dmRequest, const EventVariant& eventVariant) {
        eventBatches[dmRequest.batchId].push_back(&eventVariant);
      },
      "test_group");

  eventQueue->subscribe<EndResourceBatch>(
      [this](const EndResourceBatch& batch) { handleEndResourceBatch(batch.batchId); },
      "test_group");

  eventQueue->subscribe<UploadGeometryResponse>(
      [this](const UploadGeometryResponse& uploaded) { handleGeometryUploaded(uploaded); });

  eventQueue->subscribe<UploadImageResponse>(
      [this](const UploadImageResponse& uploaded) { handleImageUploaded(uploaded); });
}

DefaultAssetSystem::~DefaultAssetSystem() {
  Log.trace("Destroying DefaultAssetSystem");
}

auto DefaultAssetSystem::handleEndResourceBatch(uint64_t batchId) -> void {
  auto uploadPlan = UploadPlan{.stagingBuffer = stagingBufferHandle};

  for (auto& eventVariant : eventBatches[batchId]) {
    auto visitor = [&](auto&& arg) -> void {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, StaticModelRequest>) {
        Log.trace("Handling Static Model Request ID: {}", arg.requestId);
        const auto model = assetService->loadModel(arg.modelFilename);

        const auto geometryData = deInterleave(*model.staticVertices, model.indices);
        const auto uploadData = fromGeometryData(*geometryData);
        uploadPlan.uploads.insert(uploadPlan.uploads.end(), uploadData.begin(), uploadData.end());

        const auto imageUploadData = fromImageData(model.imageData);
        uploadPlan.uploads.insert(uploadPlan.uploads.end(),
                                  imageUploadData.begin(),
                                  imageUploadData.end());
      }
      if constexpr (std::is_same_v<T, DynamicModelRequest>) {
        Log.trace("Handling Dynamic Model Request ID: {}", arg.requestId);
      }
    };
    std::visit(visitor, *eventVariant);
  }
  uploadPlan.sortByBuffer();
  // TODO(matt): check staging buffer budget and handle overage somehow
  deviceBufferSystem->resizeBuffers(uploadPlan);

  /*
    - Add support for the GeometryRegionData buffer.
    - Add IBufferAllocator support to the BufferSystems so each buffer can have an
    AllocationStrategy an the BufferSystems can have a method like allocate(bufferHandle, size,
    stride) that returns an offset of where to copy the data
    - During the Allocation phase, track and fill out a GpuGeometryRegionData for each separate
    'mesh' and make sure to then allocate that and create an UploadData for it as well.
    - Memcopy the data into the staging buffer, and capture copy commands into the commandBuffer,
    'merging' copies into the same dstBuffer.
    - Submit the commandbuffer, wait on the fence, and emit all the StaticModelResponses
    - Think about the usefulness of a BatchComplete event.
    - Think about how to extend the UploadPlan to include images as well
    - Figure out how to split batches according to staging buffer size.
  */
}

auto DefaultAssetSystem::handleStaticModelRequest(const StaticModelRequest& smRequest) -> void {
  Log.trace("Handling Static Model Request ID: {}", smRequest.requestId);
}

auto DefaultAssetSystem::handleGeometryUploaded(const UploadGeometryResponse& uploaded) -> void {
  Log.trace("Handling UploadGeometryResponse Id={}, batchId={}",
            uploaded.requestId,
            uploaded.batchId);

  Log.trace("Finished Handling UploadGeometryResponse id={}", uploaded.requestId);
}

auto DefaultAssetSystem::handleImageUploaded(const UploadImageResponse& uploaded) -> void {
  Log.trace("Handling UploadImageResponse id={}, batchId={}", uploaded.requestId, uploaded.batchId);

  Log.trace("Finished Handling UploadGeometryResponse id={}", uploaded.requestId);
}

auto DefaultAssetSystem::deInterleave(const std::vector<as::StaticVertex>& vertices,
                                      const std::vector<uint32_t>& indexData)
    -> std::unique_ptr<GeometryData> {
  auto positions = std::vector<GpuVertexPositionData>{};
  auto texCoords = std::vector<GpuVertexTexCoordData>{};
  positions.resize(vertices.size());
  texCoords.resize(vertices.size());
  for (const auto& vertex : vertices) {
    positions.push_back({vertex.position});
    texCoords.push_back({vertex.texCoord});
  }
  auto indices = std::vector<GpuIndexData>{};
  indices.resize(indexData.size());
  for (const auto& index : indexData) {
    indices.push_back({index});
  }
  return std::make_unique<GeometryData>(
      GeometryData{.indexData = indices, .positionData = positions, .texCoordData = texCoords});
}

auto DefaultAssetSystem::fromGeometryData(const GeometryData& geometryData)
    -> std::vector<UploadData> {
  auto uploadDataList = std::vector<UploadData>{};
  uploadDataList.reserve(6);

  if (!geometryData.indexData.empty()) {
    auto size = geometryData.indexData.size() * sizeof(GpuIndexData);
    uploadDataList.emplace_back(
        UploadData{.dataSize = size,
                   .data = static_cast<const void*>(geometryData.indexData.data()),
                   .dstBuffer = geometryBufferPack->getIndexBuffer()});
  }

  if (!geometryData.positionData.empty()) {
    auto size = geometryData.positionData.size() * sizeof(GpuVertexPositionData);
    uploadDataList.emplace_back(
        UploadData{.dataSize = size,
                   .data = static_cast<const void*>(geometryData.positionData.data()),
                   .dstBuffer = geometryBufferPack->getPositionBuffer()});
  }

  if (!geometryData.texCoordData.empty()) {
    auto size = geometryData.texCoordData.size() * sizeof(GpuVertexTexCoordData);
    uploadDataList.emplace_back(
        UploadData{.dataSize = size,
                   .data = static_cast<const void*>(geometryData.texCoordData.data()),
                   .dstBuffer = geometryBufferPack->getTexCoordBuffer()});
  }

  return uploadDataList;
}

auto DefaultAssetSystem::fromImageData(const as::ImageData& imageData) -> std::vector<UploadData> {
  return {};
}

}
