#include "ContextFactory.hpp"

#include "DefaultContext.hpp"
#include "DefaultEventBus.hpp"

#include <di.hpp>

namespace di = boost::di;

auto ContextFactory::getContext() -> std::shared_ptr<IContext> {

   std::cout << "ContextFactory::getContext" << '\n';

   const auto injector = di::make_injector(di::bind<IEventBus>.to<DefaultEventBus>());

   return injector.create<std::shared_ptr<DefaultContext>>();
}