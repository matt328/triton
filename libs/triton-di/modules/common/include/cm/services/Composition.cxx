// composition.cpp
#include "Composition.hpp"
#include "Service.hpp"
#include "DatabaseImpl.hpp"
#include "LoggerImpl.hpp"
#include <di.hpp>
#include <utility>
#include "Bus.hpp"
#include "IThing.hpp"

namespace di = boost::di;

auto createService(std::function<std::shared_ptr<IThing>(std::shared_ptr<IBus>)> fn)
    -> std::unique_ptr<IService> {
   // Create and configure the injector
   // auto bus = std::make_shared<Bus>();
   auto injector = di::make_injector(
       di::bind<Database>.to<DatabaseImpl>(),
       di::bind<Logger>.to<LoggerImpl>(),
       di::bind<IBus>.to<Bus>(),
       di::bind<IThing>.to([&](const auto& injector) { return fn(std::move(bus)); }));

   // Create and return the fully constructed Service
   return std::make_unique<Service>(injector.create<Service>());
}
