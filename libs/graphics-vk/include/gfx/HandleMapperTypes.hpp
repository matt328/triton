#pragma once

#include "api/gfx/Geometry.hpp"
#include "bk/HandleMapper.hpp"
#include "resources/allocators/GeometryAllocator.hpp"
#include "img/Texture.hpp"

namespace tr {

using GeometryHandleMapper = HandleMapper<Geometry, GeometryRegion>;
using TextureHandleMapper = HandleMapper<TextureTag, Texture>;

}
