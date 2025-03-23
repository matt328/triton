#pragma once

namespace tr {

class ITerrainContext;

auto createTerrainContext() -> std::shared_ptr<ITerrainContext>;

}
