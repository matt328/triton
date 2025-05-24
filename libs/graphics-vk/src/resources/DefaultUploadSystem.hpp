#pragma once

#include "api/fx/ResourceEvents.hpp"
#include "gfx/IUploadSystem.hpp"

namespace tr {

class IEventQueue;
struct StaticModelRequest;
class GeometryDispatcher;

class DefaultUploadSystem : public IUploadSystem {
public:
  explicit DefaultUploadSystem(std::shared_ptr<IEventQueue> newEventQueue);
  ~DefaultUploadSystem() override = default;

  DefaultUploadSystem(const DefaultUploadSystem&) = delete;
  DefaultUploadSystem(DefaultUploadSystem&&) = delete;
  auto operator=(const DefaultUploadSystem&) -> DefaultUploadSystem& = delete;
  auto operator=(DefaultUploadSystem&&) -> DefaultUploadSystem& = delete;

private:
  std::shared_ptr<IEventQueue> eventQueue;

  std::unique_ptr<GeometryDispatcher> geometryDispatcher;

  auto handleUploadGeometry(const UploadGeometryRequest& event) -> void;
  auto handleUploadImage(const UploadImageRequest& event) -> void;
};

}
