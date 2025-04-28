#pragma once

#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"

namespace tr {

class RenderConfigRegistry;
class DrawContextFactory;

class ResourceUploader {
public:
  explicit ResourceUploader(std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
                            std::shared_ptr<DrawContextFactory> newDrawContextFactory);
  ~ResourceUploader() = default;

  ResourceUploader(const ResourceUploader&) = default;
  ResourceUploader(ResourceUploader&&) = delete;
  auto operator=(const ResourceUploader&) -> ResourceUploader& = default;
  auto operator=(ResourceUploader&&) -> ResourceUploader& = delete;

  // This method will delegate to a Factory that examines RenderableData and produces
  // GeometryBuffers, Images, MaterialBuffers ObjectDataBuffers, a RenderConfig in there somehow,
  // and a potentially cached version of a DrawContext to tie it all together.
  // It returns a RenderableHandle which is a composite of Handles to all the instances of things
  // created above
  auto registerRenderable(const RenderableData& data) -> RenderableResources;

private:
  std::shared_ptr<RenderConfigRegistry> renderConfigRegistry;
  std::shared_ptr<DrawContextFactory> drawContextFactory;
};

}
