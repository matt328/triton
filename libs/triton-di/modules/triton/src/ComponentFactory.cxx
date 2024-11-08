#include "tr/ComponentFactory.hpp"
#include "DefaultEventBus.hpp"
#include "IContext.hpp"
#include "DefaultContext.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {
   auto ComponentFactory::getContext() -> std::shared_ptr<IContext> {

      const auto injector = di::make_injector(di::bind<IEventBus>.to<DefaultEventBus>());

      return injector.create<std::shared_ptr<DefaultContext>>();
   }
}