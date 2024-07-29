#pragma once

std::ostream& operator<<(std::ostream& os, glm::mat4 const& value) {
   os << glm::to_string(value);
   return os;
}