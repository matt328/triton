#pragma once

class IService;

auto createService() -> std::unique_ptr<IService>;
