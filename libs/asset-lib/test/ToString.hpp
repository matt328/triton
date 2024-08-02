#pragma once

inline std::ostream& operator<<(std::ostream& os, glm::mat4 const& value) {
   os << to_string(value);
   return os;
}