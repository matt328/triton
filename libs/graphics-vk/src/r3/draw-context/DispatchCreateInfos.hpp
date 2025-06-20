#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {

class IShaderBinding;

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

struct CompositionContextCreateInfo {
  vk::Viewport viewport;
  vk::Rect2D scissor;
  LogicalHandle<IShaderBinding> defaultShaderBinding;
};

struct ImGuiContextCreateInfo {
  vk::Viewport viewport;
  vk::Rect2D scissor;
};

using DispatchCreateInfo = std::variant<CullingDispatchContextCreateInfo,
                                        ForwardDrawContextCreateInfo,
                                        CompositionContextCreateInfo,
                                        ImGuiContextCreateInfo>;

}
