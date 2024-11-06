// composition.cpp
#include "Composition.hpp"
#include "Service.hpp"
#include "DatabaseImpl.hpp"
#include "LoggerImpl.hpp"
#include <di.hpp>

namespace di = boost::di;

auto createService() -> std::unique_ptr<IService> {
   // Create and configure the injector
   auto injector =
       di::make_injector(di::bind<Database>.to<DatabaseImpl>(), di::bind<Logger>.to<LoggerImpl>());

   // Create and return the fully constructed Service
   return std::make_unique<Service>(injector.create<Service>());
}