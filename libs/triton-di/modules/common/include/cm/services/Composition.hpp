#pragma once

#include "IThing.hpp"
#include "IBus.hpp"

class IService;

auto createService(std::function<std::shared_ptr<IThing>(std::shared_ptr<IBus>)>)
    -> std::unique_ptr<IService>;
