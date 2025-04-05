#pragma once

namespace tr {

class ResourceUploader {
public:
  ResourceUploader() = default;
  ~ResourceUploader() = default;

  ResourceUploader(const ResourceUploader&) = default;
  ResourceUploader(ResourceUploader&&) = delete;
  auto operator=(const ResourceUploader&) -> ResourceUploader& = default;
  auto operator=(ResourceUploader&&) -> ResourceUploader& = delete;

  auto registerRenderable() -> RenderableHandle;
};

}
