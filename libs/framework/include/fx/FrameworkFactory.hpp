#pragma once

#include "FrameworkConfig.hpp"

namespace tr {

class IFrameworkContext;

auto createFrameworkContext(const FrameworkConfig& config, std::shared_ptr<IGuiAdapter> guiAdapter)
    -> std::shared_ptr<IFrameworkContext>;

}
