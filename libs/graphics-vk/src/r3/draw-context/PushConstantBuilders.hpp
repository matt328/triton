#pragma once

#include "r3/draw-context/PushConstantBlob.hpp"

namespace tr {

struct DispatchContextConfig;
struct DrawContextConfig;
class Frame;

using DispatchPushConstantsBuilder =
    std::function<PushConstantBlob(const DispatchContextConfig&, const Frame&)>;

using DrawPushConstantsBuilder =
    std::function<PushConstantBlob(const DrawContextConfig&, const Frame&)>;
}
