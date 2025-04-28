#pragma once

namespace tr {

class Frame;
using PushConstantsBindFn = std::function<void(const Frame*, vk::raii::CommandBuffer&)>;

}
