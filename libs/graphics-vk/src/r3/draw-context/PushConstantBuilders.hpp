#pragma once

#include "r3/draw-context/PushConstantBlob.hpp"

namespace tr {

struct DispatchContextConfig;
class Frame;

using DispatchPushConstantsBuilder =
    std::function<PushConstantBlob(const DispatchContextConfig&, const Frame&)>;
}
