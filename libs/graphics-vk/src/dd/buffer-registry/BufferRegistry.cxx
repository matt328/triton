#include "dd/buffer-registry/BufferRegistry.hpp"

namespace tr {

BufferRegistry::BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager)
    : bufferManager{std::move(newBufferManager)} {
}

auto BufferRegistry::getOrCreate([[maybe_unused]] const BufferKey& key) -> BufferHandle {
  return static_cast<BufferHandle>(0L);
}

}
