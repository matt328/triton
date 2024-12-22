#pragma once

#include <string>
namespace tr {
   class Resource {
    public:
      Resource() = default;
      ~Resource() = default;

      Resource(const Resource&) = default;
      Resource(Resource&&) = delete;
      auto operator=(const Resource&) -> Resource& = default;
      auto operator=(Resource&&) -> Resource& = delete;

    private:
      std::string id;
   };
}
