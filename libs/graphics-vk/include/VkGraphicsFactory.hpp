#pragma once

namespace tr {

class IGraphicsContext;
class IGuiCallbackRegistrar;

auto createVkGraphicsContext(std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar)
    -> std::shared_ptr<IGraphicsContext>;

}
