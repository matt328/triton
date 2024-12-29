#pragma once

inline static unsigned int factorial(unsigned int number) {
  return number <= 1 ? number : factorial(number - 1) * number;
}
