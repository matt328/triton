#include "fx/FrameworkFactory.hpp"
#include "FixedGameLoop.hpp"
#include "FrameworkContextImpl.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createFrameworkContext([[maybe_unused]] const FrameworkConfig& config)
    -> std::shared_ptr<IFrameworkContext> {
  const auto injector = di::make_injector(di::bind<IGameLoop>.to<FixedGameLoop>());

  return injector.create<std::shared_ptr<FrameworkContextImpl>>();
}

}
