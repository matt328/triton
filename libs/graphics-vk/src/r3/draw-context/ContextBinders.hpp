#pragma once

#include "r3/draw-context/PushConstantBlob.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace tr {

struct DispatchContextConfig;
struct DrawContextConfig;
class Frame;

using DispatchBinder =
    std::function<void(const DispatchContextConfig&, const Frame&, vk::CommandBuffer&)>;

using DrawBinder = std::function<(const DrawContextConfig&, const Frame&)>;
}
