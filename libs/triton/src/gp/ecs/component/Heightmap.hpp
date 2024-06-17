#pragma once

namespace tr::gp::ecs {
   struct Heightmap {
      uint32_t width;
      std::vector<uint32_t> data;
   };
}
