#pragma once

#include "bk/HandleGenerator.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "buffers/UploadPlan.hpp"

namespace tr {

class Device;
class Allocator;
class FrameState;
struct DeviceBufferCreateInfo;
class IBufferAllocator;

class DeviceBufferSystem {
public:
  DeviceBufferSystem(std::shared_ptr<Device> newDevice,
                     std::shared_ptr<Allocator> newAllocator,
                     std::shared_ptr<FrameState> newFrameState);
  ~DeviceBufferSystem();

  DeviceBufferSystem(const DeviceBufferSystem&) = delete;
  DeviceBufferSystem(DeviceBufferSystem&&) = delete;
  auto operator=(const DeviceBufferSystem&) -> DeviceBufferSystem& = delete;
  auto operator=(DeviceBufferSystem&&) -> DeviceBufferSystem& = delete;

  auto resizeBuffers(const UploadPlan& uploadPlan) -> void;
  auto registerBuffer(const DeviceBufferCreateInfo& createInfo) -> Handle<ManagedBuffer>;
  auto tryInsert(Handle<ManagedBuffer> handle, const void* data, size_t size)
      -> std::optional<BufferRegion>;
  auto removeData(Handle<ManagedBuffer> handle, const BufferRegion& region) -> void;
  auto getBufferAddress(Handle<ManagedBuffer> handle) -> std::optional<uint64_t>;
  auto getVkBuffer(Handle<ManagedBuffer> handle) -> std::optional<vk::Buffer>;
  auto endFrame() -> void;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<FrameState> frameState;

  struct BufferEntry {
    std::deque<std::unique_ptr<ManagedBuffer>> versions;
  };

  HandleGenerator<ManagedBuffer> generator;
  std::unordered_map<Handle<ManagedBuffer>, std::unique_ptr<BufferEntry>> bufferMap;
  std::unordered_map<Handle<ManagedBuffer>, std::unique_ptr<IBufferAllocator>> allocatorMap;

  static auto fromCreateInfo(const DeviceBufferCreateInfo& createInfo)
      -> std::tuple<vk::BufferCreateInfo, vma::AllocationCreateInfo>;
};

}
