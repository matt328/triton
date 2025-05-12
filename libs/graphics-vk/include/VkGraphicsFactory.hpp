#pragma once

#include "VkGraphicsCreateInfo.hpp"

namespace tr {

class IGraphicsContext;
class IGuiCallbackRegistrar;
class IEventBus;
class TaskQueue;
class IGuiAdapter;
class IStateBuffer;
class IWindow;

auto createVkGraphicsContext(VkGraphicsCreateInfo createInfo,
                             std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                             std::shared_ptr<IEventBus> newEventBus,
                             std::shared_ptr<TaskQueue> newTaskQueue,
                             std::shared_ptr<IStateBuffer> newStateBuffer,
                             std::shared_ptr<IWindow> newWindow)
    -> std::shared_ptr<IGraphicsContext>;

}
