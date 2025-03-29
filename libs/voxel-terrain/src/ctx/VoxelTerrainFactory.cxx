#include "VoxelTerrainFactory.hpp"
#include "VoxelTerrainContext.hpp"
#include "VoxelTerrainSystemProxy.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createTerrainContext() -> std::shared_ptr<ITerrainContext> {

  const auto injector =
      di::make_injector(di::bind<ITerrainSystemProxy>.to<VoxelTerrainSystemProxy>());

  return injector.create<std::shared_ptr<VoxelTerrainContext>>();
}
}
