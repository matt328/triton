#pragma once

#include "IThing.hpp"

class IService;

auto createService(std::function<std::shared_ptr<IThing>(void)>) -> std::unique_ptr<IService>;
