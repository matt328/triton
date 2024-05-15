#include "gfx/textures/ResourceManager.hpp"
#include "ct/HeightField.hpp"
#include "gfx/Handles.hpp"
#include "gfx/RenderData.hpp"
#include "gfx/geometry/GeometryFactory.hpp"
#include "gfx/geometry/GeometryHandles.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

#include "gfx/GraphicsDevice.hpp"
#include "gfx/VkContext.hpp"

#include "gfx/geometry/GeometryData.hpp"
#include "gfx/geometry/Mesh.hpp"

#include "gfx/mem/Allocator.hpp"
#include "gfx/mem/Buffer.hpp"

namespace tr::gfx::tx {
   ResourceManager::ResourceManager(const GraphicsDevice& graphicsDevice)
       : graphicsDevice{graphicsDevice} {
      taskQueue = std::make_unique<util::TaskQueue>();
      geometryFactory = std::make_unique<geo::GeometryFactory>();
   }

   ResourceManager::~ResourceManager() {
   }

   void ResourceManager::setRenderData(RenderData& newRenderData) {
      std::lock_guard<LockableBase(std::mutex)> lock(renderDataMutex);
      LockableName(renderDataMutex, "SetRenderData", 13);
      LockMark(renderDataMutex);
      renderData = newRenderData;
   }

   void ResourceManager::accessRenderData(std::function<void(RenderData&)> fn) {
      std::lock_guard<LockableBase(std::mutex)> lock(renderDataMutex);
      LockableName(renderDataMutex, "AccessRenderData", 16);
      LockMark(renderDataMutex);
      fn(renderData);
   }

   auto ResourceManager::createTerrain() -> std::future<ModelHandle> {
      return taskQueue->enqueue([this]() { return createTerrainInt(); });
   }

   auto ResourceManager::createTerrainInt() -> ModelHandle {
      const auto heightfield = ct::HeightField{1024};
      const auto dataHandle = geometryFactory->createGeometryFromHeightfield(heightfield);
      const auto modelHandle = uploadGeometry(dataHandle);
      geometryFactory->unload(dataHandle);
      return modelHandle;
   }

   std::future<ModelHandle> ResourceManager::loadModelAsync(const std::filesystem::path& filename) {
      return taskQueue->enqueue([this, filename]() { return loadModelInt(filename); });
   }

   ModelHandle ResourceManager::loadModelInt(const std::filesystem::path& filename) {
      const auto texturedGeometryHandle = geometryFactory->loadGeometryFromGltf(filename);
      return uploadGeometry(texturedGeometryHandle);
   }

   std::future<SkinnedModelHandle> ResourceManager::loadSkinnedModelAsync(
       const std::string_view modelPath,
       const std::string_view skeletonPath,
       const std::string_view animationPath) {
      return taskQueue->enqueue([this, modelPath, skeletonPath, animationPath]() {
         return loadSkinnedModelInt(modelPath, skeletonPath, animationPath);
      });
   }

   SkinnedModelHandle ResourceManager::loadSkinnedModelInt(const std::string_view modelPath,
                                                           const std::string_view skeletonPath,
                                                           const std::string_view animationPath) {
      const auto sgd = geometryFactory->loadSkinnedModel(modelPath, skeletonPath, animationPath);
      return uploadSkinnedGeometry(sgd);
   }

   /// Uploads the Geometry and Texture data and creates space in a buffer for the skeleton's
   /// joint matrix data
   auto ResourceManager::uploadSkinnedGeometry(const geo::SkinnedGeometryData& sgd)
       -> SkinnedModelHandle {
      const auto tgh = geo::TexturedGeometryHandle{{sgd.geometryHandle, sgd.imageHandle}};
      const auto modelHandle = uploadGeometry(tgh);

      /*
         JoinMatrices are a new type of data since they get written into a shared buffer, and also
         get updated each frame.

         - The skeleton and animation data structures are used by the ECS on
         the CPU side to produce a set of JointMatrices for each skinned model.
         - Each JointMatrices set needs added to a list in RenderData, along with adding the
         indexing information to the SkinnedModelHandles in RenderData
         - At the sync point, Renderer will lock the RenderData structure and copy indexing strucure
         list and upload JointMatrices to a cpu->gpu buffer (host visible, host coherent)
         - This method should return the mesh and texture handles along with the Skeleton and
         Animation data.
         - The mesh and texture handles can be put in a Renderable component, and the Skeleton and
         Animation put into an animation Component.
         - During AnimationSystem update, joint matrices will be calculated and stored in the
         component.
         - During RenderDataSystem update, if the entity has an Animation component, add it's
         jointmatrices to the renderdata list, and calculate its indexing structure and add it to
         renderdata.skinnedMeshData
      */
      const auto smh = SkinnedModelHandle{

      };

      return smh;
   }

   /*
      TODO: Consider making ResourceManager's data either aggregate types that the ResourceManager
      is responsible for knowing how to construct, or make them classes that know how to construct
      themselves. Aggregate types moves a lot of logic into the ResourceManager, but classes need
      alot of dependencies passed into their constructors.  Probably factories that produce
      aggregate types might be a good compromise, that way the factories could encapsulate the logic
      instead of ResourceManager. Factories can use RVO to return aggregate types by value and avoid
      a copy.
   */
   auto ResourceManager::uploadGeometry(const geo::TexturedGeometryHandle& handles) -> ModelHandle {
      auto modelHandles = ModelHandle{};
      auto& allocator = graphicsDevice.getAllocator();
      auto& context = graphicsDevice.getAsyncTransferContext();

      for (const auto& [geometryHandle, imageHandle] : handles) {
         const auto meshHandle = meshList.size();

         meshList.emplace_back(geo::Mesh{});
         auto& mesh = meshList.back();

         const auto geometryData = geometryFactory->getGeometryData(geometryHandle);

         // Prepare Vertex Buffer
         const auto vbSize = geometryData.vertexDataSize();
         const auto vbStagingBuffer =
             allocator.createStagingBuffer(vbSize, "Vertex Staging Buffer");

         void* vbData = allocator.mapMemory(*vbStagingBuffer);
         memcpy(vbData, geometryData.vertices.data(), static_cast<size_t>(vbSize));
         allocator.unmapMemory(*vbStagingBuffer);

         mesh.vertexBuffer = allocator.createGpuVertexBuffer(vbSize, "GPU Vertex");

         // Prepare Index Buffer
         const auto ibSize = geometryData.indexDataSize();
         const auto ibStagingBuffer = allocator.createStagingBuffer(ibSize, "Index Staging Buffer");

         auto data = allocator.mapMemory(*ibStagingBuffer);
         memcpy(data, geometryData.indices.data(), static_cast<size_t>(ibSize));
         allocator.unmapMemory(*ibStagingBuffer);

         mesh.indexBuffer = allocator.createGpuIndexBuffer(ibSize, "GPU Index");
         mesh.indicesCount = geometryData.indices.size();

         // Upload Buffers
         context.submit([&](const vk::raii::CommandBuffer& cmd) {
            const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = vbSize};
            cmd.copyBuffer(vbStagingBuffer->getBuffer(), mesh.vertexBuffer->getBuffer(), vbCopy);
            const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = ibSize};
            cmd.copyBuffer(ibStagingBuffer->getBuffer(), mesh.indexBuffer->getBuffer(), copy);
         });

         const auto image = geometryFactory->getImageData(imageHandle);
         const auto textureHandle = textureList.size();
         textureList.emplace_back(
             std::make_unique<Textures::Texture>((void*)image.data.data(),
                                                 image.width,
                                                 image.height,
                                                 image.component,
                                                 allocator,
                                                 graphicsDevice.getVulkanDevice(),
                                                 context));

         { // Only need to guard access to the textureInfoList
            ZoneNamedN(c, "Update TextureInfoList", true);
            std::lock_guard<LockableBase(std::mutex)> lock(textureListMutex);
            LockMark(textureListMutex);
            LockableName(textureListMutex, "Mutate", 6);
            textureInfoList.emplace_back(textureList[textureHandle]->getImageInfo());
         }
         modelHandles.insert({meshHandle, textureHandle});
      }

      return modelHandles;
   }

   void ResourceManager::accessTextures(
       std::function<void(const std::vector<vk::DescriptorImageInfo>&)> fn) const {
      if (textureInfoList.empty()) {
         return;
      }
      std::lock_guard<LockableBase(std::mutex)> lock(textureListMutex);
      LockableName(textureListMutex, "Access", 6);
      LockMark(textureListMutex);
      fn(textureInfoList);
   }
}