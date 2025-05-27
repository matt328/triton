#pragma once

#include "api/fx/ResourceEvents.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "gfx/IUploadSystem.hpp"

namespace tr {

struct UploadInfo {
  void* srcData;
  size_t size;
  Handle<ManagedBuffer> dstBuffer;
};

class IEventQueue;
struct StaticModelRequest;
class GeometryDispatcher;
class DeviceBufferSystem;

class DefaultUploadSystem : public IUploadSystem {
public:
  explicit DefaultUploadSystem(std::shared_ptr<IEventQueue> newEventQueue,
                               std::shared_ptr<DeviceBufferSystem> newDeviceBufferSystem);
  ~DefaultUploadSystem() override = default;

  DefaultUploadSystem(const DefaultUploadSystem&) = delete;
  DefaultUploadSystem(DefaultUploadSystem&&) = delete;
  auto operator=(const DefaultUploadSystem&) -> DefaultUploadSystem& = delete;
  auto operator=(DefaultUploadSystem&&) -> DefaultUploadSystem& = delete;

private:
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<DeviceBufferSystem> bufferSystem;

  std::unique_ptr<GeometryDispatcher> geometryDispatcher;

  auto handleUploadGeometry(const UploadGeometryRequest& event) -> void;
  auto handleUploadImage(const UploadImageRequest& event) -> void;

  auto splitGeometry(const UploadGeometryRequest& event) -> std::vector<UploadInfo>;
};

}
