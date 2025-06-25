#pragma once

#include "api/gw/EditorState.hpp"

namespace tr {

class EditorSystem {
public:
  EditorSystem() = default;
  ~EditorSystem() = default;

  EditorSystem(const EditorSystem&) = default;
  EditorSystem(EditorSystem&&) = delete;
  auto operator=(const EditorSystem&) -> EditorSystem& = default;
  auto operator=(EditorSystem&&) -> EditorSystem& = delete;

  static auto update(entt::registry& registry) -> const EditorState&;
};

}
