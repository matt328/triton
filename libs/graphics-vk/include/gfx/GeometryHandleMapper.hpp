#pragma once

#include "api/gfx/Geometry.hpp"
#include "bk/HandleMapper.hpp"
#include "resources/allocators/GeometryAllocator.hpp"

namespace tr {

using GeometryHandleMapper = HandleMapper<Geometry, GeometryRegion>;

}
