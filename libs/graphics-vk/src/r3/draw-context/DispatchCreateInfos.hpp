#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {
struct CullingDispatchContextCreateInfo {
  LogicalHandle<ManagedBuffer> resourceTable;
  LogicalHandle<ManagedBuffer> frameData;
};

struct ForwardDrawContextCreateInfo {
  vk::Viewport viewport;
  vk::Rect2D scissor;
  LogicalHandle<ManagedBuffer> resourceTable;
  LogicalHandle<ManagedBuffer> frameData;
  LogicalHandle<ManagedBuffer> indirectCommand;
  LogicalHandle<ManagedBuffer> indirectCommandCount;
};

struct CompositionContextCreateInfo {};

using DispatchCreateInfo = std::variant<CullingDispatchContextCreateInfo,
                                        ForwardDrawContextCreateInfo,
                                        CompositionContextCreateInfo>;

}
