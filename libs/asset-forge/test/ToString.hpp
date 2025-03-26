#pragma once

inline auto operator<<(std::ostream& os, glm::mat4 const& value) -> std::ostream& {
  os << to_string(value);
  return os;
}
