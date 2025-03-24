#pragma once

namespace tr {

class IGraphicsContext;
class IGuiCallbackRegistrar;
class IEventBus;
class ITaskQueue;
class IGuiAdapter;

auto createVkGraphicsContext(std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                             const std::shared_ptr<IEventBus>& newEventBus,
                             std::shared_ptr<ITaskQueue> newTaskQueue,
                             const std::shared_ptr<IGuiAdapter>& newGraphicsAdapter)
    -> std::shared_ptr<IGraphicsContext>;

}
