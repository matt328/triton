#pragma once

// Helper: compare vectors
template <typename T>
auto vecEqual(const std::vector<T>& a, const std::vector<T>& b) -> bool {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

// Helper: hash combine
template <typename T>
inline void hash_combine(std::size_t& seed, const T& val) {
  seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
