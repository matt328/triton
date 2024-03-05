#pragma once

#include "entt/fwd.hpp"
#include "ctx/GameplayFacade.hpp"

namespace ed::io {
   void writeProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade);
   void readProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade);
}