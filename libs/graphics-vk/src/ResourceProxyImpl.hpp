#pragma once

#include "api/fx/IResourceProxy.hpp"

namespace tr {

class VkResourceManager;

class ResourceProxyImpl : public IResourceProxy {
public:
  explicit ResourceProxyImpl(std::shared_ptr<VkResourceManager> newResourceManager);
  ~ResourceProxyImpl() override = default;

  ResourceProxyImpl(const ResourceProxyImpl&) = default;
  ResourceProxyImpl(ResourceProxyImpl&&) = delete;
  auto operator=(const ResourceProxyImpl&) -> ResourceProxyImpl& = default;
  auto operator=(ResourceProxyImpl&&) -> ResourceProxyImpl& = delete;

  auto uploadModel(const as::Model& model) -> ModelData override;

  auto uploadGeometry(const DDGeometryData& data) -> MeshHandle override;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
};

}
