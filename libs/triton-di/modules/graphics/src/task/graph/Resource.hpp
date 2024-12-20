#pragma once

namespace tr::gfx::task::graph {
   class Resource {
    public:
      Resource();
      ~Resource();

      Resource(const Resource&) = default;
      Resource(Resource&&) = delete;
      auto operator=(const Resource&) -> Resource& = default;
      auto operator=(Resource&&) -> Resource& = delete;
   };
}
