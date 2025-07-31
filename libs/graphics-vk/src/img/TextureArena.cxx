#include "TextureArena.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/Texture.hpp"
#include "task/Frame.hpp"
#include "vk/sb/IShaderBinding.hpp"
#include "vk/sb/IShaderBindingFactory.hpp"

namespace tr {

TextureArena::TextureArena(std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                           std::shared_ptr<DSLayoutManager> newLayoutManager)
    : shaderBindingFactory{std::move(newShaderBindingFactory)},
      layoutManager{std::move(newLayoutManager)} {

  const auto defaultLayout = vk::DescriptorSetLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 256,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr,
  };

  dsLayout = layoutManager->createLayout(defaultLayout, "DefaultLayoutBinding");
  shaderBinding = shaderBindingFactory->createShaderBinding(ShaderBindingType::Textures, dsLayout);
}

auto TextureArena::insert(vk::ImageView imageView, vk::Sampler sampler) -> Handle<Texture> {
  std::scoped_lock lock(swapMutex);
  const auto handle = textureHandleGenerator.requestHandle();
  const auto position = textures.size();
  stagingTextures.push_back({.view = imageView, .sampler = sampler});
  stagingHandleMap.emplace(handle, position);

  for (auto& [key, value] : dirty) {
    value = true;
  }

  newDataAvailable = true;
  return handle;
}

auto TextureArena::remove(Handle<Texture> handle) -> void {
  for (auto& [key, value] : dirty) {
    value = true;
  }
}

auto TextureArena::updateShaderBindings(const Frame* frame) -> void {
  if (newDataAvailable.exchange(false)) {
    std::scoped_lock lock(swapMutex);

    for (const auto& tex : stagingTextures) {
      textures.emplace_back(tex);
    }

    for (const auto& [key, value] : stagingHandleMap) {
      handleMap[key] = value;
    }

    stagingTextures.clear();
    stagingHandleMap.clear();
    for (auto& [key, value] : dirty) {
      value = true;
    }
  }

  if ((!dirty.contains(frame->getIndex()) || dirty.at(frame->getIndex())) && !textures.empty()) {
    auto imageInfoList = std::vector<vk::DescriptorImageInfo>{};
    for (const auto& texture : textures) {
      imageInfoList.push_back({
          .sampler = texture.sampler,
          .imageView = texture.view,
          .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      });
    }
    auto handle = frame->getLogicalShaderBinding(shaderBinding);
    auto& shaderBinding = shaderBindingFactory->getShaderBinding(handle);
    shaderBinding.bindImageSamplers(0, imageInfoList);
    dirty.insert_or_assign(frame->getIndex(), false);
  }
}

/// Must only be called *after* updateShaderBindings
auto TextureArena::getTextureIndex(Handle<Texture> handle) -> uint32_t {
  assert(handleMap.contains(handle));
  return handleMap.at(handle);
}

auto TextureArena::getDSLayoutHandle() const -> Handle<DSLayout> {
  return dsLayout;
}

auto TextureArena::bindShaderBindings(const Frame* frame,
                                      vk::raii::CommandBuffer& cmdBuffer,
                                      vk::PipelineLayout pipelineLayout) -> void {

  auto& binding =
      shaderBindingFactory->getShaderBinding(frame->getLogicalShaderBinding(shaderBinding));
  binding.bindToPipeline(cmdBuffer, vk::PipelineBindPoint::eGraphics, 0, pipelineLayout);
}

}
