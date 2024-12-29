#include "Resource.hpp"

namespace tr {

auto Resource::operator==(const Resource& other) const -> bool {
  return id == other.id;
}

}

