#pragma once

#include "api/fx/ResourceEvents.hpp"
#include "gfx/IUploadSystem.hpp"

namespace tr {

class IEventQueue;
struct StaticModelRequest;

class DefaultUploadSystem : public IUploadSystem {
public:
  explicit DefaultUploadSystem(std::shared_ptr<IEventQueue> newEventQueue);
  ~DefaultUploadSystem() override = default;

  DefaultUploadSystem(const DefaultUploadSystem&) = default;
  DefaultUploadSystem(DefaultUploadSystem&&) = delete;
  auto operator=(const DefaultUploadSystem&) -> DefaultUploadSystem& = default;
  auto operator=(DefaultUploadSystem&&) -> DefaultUploadSystem& = delete;

private:
  std::shared_ptr<IEventQueue> eventQueue;

  auto handleUploadGeometry(const UploadGeometryRequest& event) -> void;
};

}
