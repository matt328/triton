#include "ResourceProxyImpl.hpp"

#include "VkResourceManager.hpp"

namespace tr {

ResourceProxyImpl::ResourceProxyImpl(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)} {
}

auto ResourceProxyImpl::uploadModel([[maybe_unused]] const as::Model& model) -> ModelData {
  // return resourceManager->uploadModel(model);
  return ModelData{};
}

}
