#pragma once

#include "entt/fwd.hpp"

namespace ed::io {
   void writeProjectFile(std::string_view filename);
   void readProjectFile(entt::registry& registry, std::string_view& filename);
}