#include "IndexAllocator.hpp"

namespace tr {

IndexAllocator::IndexAllocator(Handle<ManagedBuffer> destinationBuffer)
    : indexBuffer{destinationBuffer} {
}

auto IndexAllocator::allocate(uint64_t resourceId, const UploadRequestVariant& req)
    -> std::optional<MeshBufferRegion> {
}

auto IndexAllocator::reset() -> void {
}

}
