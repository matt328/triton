#pragma once

#include "api/fx/IResourceProxy.hpp"
#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"

namespace tr {

class VkResourceManager;
class IRenderContext;

class ResourceProxyImpl : public IResourceProxy {
public:
  explicit ResourceProxyImpl(std::shared_ptr<VkResourceManager> newResourceManager,
                             std::shared_ptr<IRenderContext> newRenderContext);
  ~ResourceProxyImpl() override = default;

  ResourceProxyImpl(const ResourceProxyImpl&) = default;
  ResourceProxyImpl(ResourceProxyImpl&&) = delete;
  auto operator=(const ResourceProxyImpl&) -> ResourceProxyImpl& = default;
  auto operator=(ResourceProxyImpl&&) -> ResourceProxyImpl& = delete;

  auto uploadModel(const as::Model& model) -> ModelData override;
  auto uploadGeometry(DDGeometryData&& data) -> MeshHandle override;
  auto registerRenderable(RenderableData&& data) -> RenderableResources override;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IRenderContext> renderContext;
};

}
