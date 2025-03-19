#include "VkGraphicsFactory.hpp"

#include "VkGraphicsContext.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {
auto createVkGraphicsContext(std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar)
    -> std::shared_ptr<IGraphicsContext> {
  const auto injector =
      di::make_injector(di::bind<IGuiCallbackRegistrar>.to(newGuiCallbackRegistrar));

  return injector.create<std::shared_ptr<VkGraphicsContext>>();
}

}
