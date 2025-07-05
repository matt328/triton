#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "img/Texture.hpp"

namespace tr {

class IShaderBinding;
class IShaderBindingFactory;
class IFrameManager;
class DSLayoutManager;
class DSLayout;
class Frame;

class TextureArena {
public:
  TextureArena(std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
               std::shared_ptr<DSLayoutManager> newLayoutManager);
  ~TextureArena() = default;

  TextureArena(const TextureArena&) = delete;
  TextureArena(TextureArena&&) = delete;
  auto operator=(const TextureArena&) -> TextureArena& = delete;
  auto operator=(TextureArena&&) -> TextureArena& = delete;

  auto insert(vk::ImageView imageView, vk::Sampler sampler) -> Handle<Texture>;
  auto remove(Handle<Texture> handle) -> void;
  auto updateShaderBindings(const Frame* frame) -> void;
  auto getTextureIndex(Handle<Texture> handle) -> uint32_t;
  auto getDSLayoutHandle() const -> Handle<DSLayout>;
  auto bindShaderBindings(const Frame* frame,
                          vk::raii::CommandBuffer& cmdBuffer,
                          vk::PipelineLayout pipelineLayout) -> void;

private:
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  std::shared_ptr<DSLayoutManager> layoutManager;

  HandleGenerator<Texture> textureHandleGenerator{};

  std::unordered_map<uint32_t, bool> dirty;
  std::unordered_map<Handle<Texture>, size_t> handleMap;
  std::vector<Texture> textures;

  Handle<DSLayout> dsLayout;
  LogicalHandle<IShaderBinding> shaderBinding;

  std::mutex swapMutex;
  std::vector<Texture> stagingTextures;
  std::unordered_map<Handle<Texture>, size_t> stagingHandleMap;
  std::atomic_bool newDataAvailable = false;
};

}
