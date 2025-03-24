#pragma once

#include "VkGraphicsCreateInfo.hpp"

namespace tr {

class IGraphicsContext;
class IGuiCallbackRegistrar;
class IEventBus;
class ITaskQueue;
class IGuiAdapter;

auto createVkGraphicsContext(const VkGraphicsCreateInfo& createInfo,
                             std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                             const std::shared_ptr<IEventBus>& newEventBus,
                             std::shared_ptr<ITaskQueue> newTaskQueue,
                             const std::shared_ptr<IGuiAdapter>& newGuiAdapter)
    -> std::shared_ptr<IGraphicsContext>;

}
