#include <di.hpp>

#include "ContextFactory.hpp"
#include "IContext.hpp"
#include "IWindow.hpp"
#include "src/Application.hpp"
#include "src/DefaultWindow.hpp"

namespace di = boost::di;

auto main() -> int {

   auto context = ContextFactory::getContext();

   const auto injector =
       di::make_injector(di::bind<IWindow>.to<DefaultWindow>(), di::bind<IContext>.to(context));

   auto app = injector.create<std::shared_ptr<Application>>();

   return 0;
}
