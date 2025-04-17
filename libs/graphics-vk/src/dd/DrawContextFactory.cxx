#include "dd/DrawContextFactory.hpp"
#include "dd/DrawContext.hpp"
#include "dd/RenderConfigRegistry.hpp"
#include "dd/buffer-registry/BufferRegistry.hpp"
#include "dd/gpu-data/GpuMaterialData.hpp"
#include "gfx/IFrameManager.hpp"
#include "vk/BufferEntry.hpp"

namespace tr {

DrawContextFactory::DrawContextFactory(
    std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
    std::shared_ptr<BufferRegistry> newBufferRegistry,
    std::shared_ptr<IFrameManager> newFrameManager)
    : renderConfigRegistry{std::move(newRenderConfigRegistry)},
      bufferRegistry{std::move(newBufferRegistry)},
      frameManager{std::move(newFrameManager)} {
}

auto DrawContextFactory::getOrCreateDrawContext(RenderConfigHandle renderConfigHandle)
    -> DrawContext* {

  if (!drawContexts.contains(renderConfigHandle)) {
    const auto renderConfig = renderConfigRegistry->get(renderConfigHandle);

    const auto drawContextId = drawContextKeygen.getKey();

    // Geometry Buffer
    const auto instance = BufferInstanceKey{
        .drawContextId = 0,
        .frameId = 0,
    };
    const auto profile =
        BufferUsageProfile{.kind = BufferKind::Geometry,
                           .usages = {BufferUsage::GeometryIndex, BufferUsage::GeometryVertex},
                           .debugName = "geometrybuffer",
                           .stride = renderConfig.vertexFormat.stride,
                           .maxElements = 10240,
                           .extra = GeometryExtraProfile{
                               .vertexFormat = renderConfig.vertexFormat,
                               .indexSize = sizeof(uint32_t),
                           }};
    const auto geometryBufferHandle =
        bufferRegistry->getOrCreate(BufferKey{.profile = profile, .instance = instance});

    // Material Buffer
    const auto materialBufferInstance = BufferInstanceKey{
        .drawContextId = 0,
        .frameId = 0,
    };
    const auto materialProfile = BufferUsageProfile{
        .kind = BufferKind::Material,
        .usages = {BufferUsage::CpuWritable,
                   BufferUsage::ShaderDeviceAddress,
                   BufferUsage::Storage},
        .debugName = "Materials",
        .stride = sizeof(GpuMaterialData),
        .maxElements = 128,
    };
    const auto materialBufferHandle = bufferRegistry->getOrCreate(
        BufferKey{.profile = materialProfile, .instance = materialBufferInstance});

    // GeometryRegion Buffer
    const auto regionInstance = BufferInstanceKey{.drawContextId = 0, .frameId = 0};
    const auto regionProfile =
        BufferUsageProfile{.kind = BufferKind::GpuBufferRegion,
                           .usages = {BufferUsage::CpuWritable,
                                      BufferUsage::ShaderDeviceAddress,
                                      BufferUsage::Storage},
                           .debugName = std::format("{}-GeometryRegion", drawContextId),
                           .stride = sizeof(GpuGeometryRegionData),
                           .maxElements = 128};
    const auto geometryRegionBufferHandle = bufferRegistry->getOrCreate(
        BufferKey{.profile = regionProfile, .instance = regionInstance});

    // Object Buffer
    const auto objectBufferProfile =
        BufferUsageProfile{.kind = BufferKind::GpuBufferRegion,
                           .usages = {BufferUsage::CpuWritable,
                                      BufferUsage::ShaderDeviceAddress,
                                      BufferUsage::Storage},
                           .debugName = std::format("{}-ObjectBuffer", drawContextId),
                           .stride = sizeof(GpuObjectData),
                           .maxElements = 128};
    const auto logicalObjectBufferHandle =
        frameManager->createPerFrameBuffer(objectBufferProfile, drawContextId);

    // Object Count Buffer
    const auto objectCountBufferProfile =
        BufferUsageProfile{.kind = BufferKind::GpuBufferRegion,
                           .usages = {BufferUsage::CpuWritable,
                                      BufferUsage::ShaderDeviceAddress,
                                      BufferUsage::Storage},
                           .debugName = std::format("{}-ObjectCount", drawContextId),
                           .stride = sizeof(GpuObjectCountData),
                           .maxElements = 1};
    const auto logicalObjectCountBufferHandle =
        frameManager->createPerFrameBuffer(objectCountBufferProfile, drawContextId);

    // Object Index Buffer
    const auto objectIndexBufferProfile =
        BufferUsageProfile{.kind = BufferKind::GpuBufferRegion,
                           .usages = {BufferUsage::CpuWritable,
                                      BufferUsage::ShaderDeviceAddress,
                                      BufferUsage::Storage},
                           .debugName = std::format("{}-ObjectIndex", drawContextId),
                           .stride = sizeof(GpuObjectCountData),
                           .maxElements = 128};
    const auto logicalObjectIndexBufferHandle =
        frameManager->createPerFrameBuffer(objectIndexBufferProfile, drawContextId);

    // IndirectDrawCommand Buffer
    const auto indirectDrawBufferProfile =
        BufferUsageProfile{.kind = BufferKind::IndirectCommand,
                           .usages = {BufferUsage::CpuWritable,
                                      BufferUsage::ShaderDeviceAddress,
                                      BufferUsage::Indirect},
                           .debugName = std::format("{}-Indirect", drawContextId),
                           .stride = sizeof(GpuIndirectCommand),
                           .maxElements = 128};
    const auto logicalIndirectBufferHandle =
        frameManager->createPerFrameBuffer(indirectDrawBufferProfile, drawContextId);

    const auto dcci =
        DrawContextCreateInfo{.id = drawContextId,
                              .geometryBufferHandle = geometryBufferHandle,
                              .materialBufferHandle = materialBufferHandle,
                              .geometryRegionBufferHandle = geometryRegionBufferHandle,
                              .objectDataBufferHandle = logicalObjectBufferHandle,
                              .objectDataIndexBufferHandle = logicalObjectIndexBufferHandle,
                              .objectCountBufferHandle = logicalObjectCountBufferHandle,
                              .indirectDrawCommandBufferHandle = logicalIndirectBufferHandle};

    drawContexts.emplace(renderConfigHandle, std::make_unique<DrawContext>(dcci));
  }

  return drawContexts.at(renderConfigHandle).get();
}
}
