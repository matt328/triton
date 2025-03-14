#pragma once

inline void hash_combine(std::size_t& seed, std::size_t hash) {
  seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
template <>
struct hash<glm::vec2> {
  auto operator()(const glm::vec2& vec) const -> std::size_t {
    std::size_t seed = 0;
    std::hash<float> hasher;
    hash_combine(seed, hasher(vec.x));
    hash_combine(seed, hasher(vec.y));
    return seed;
  }
};

template <>
struct hash<glm::vec3> {
  auto operator()(const glm::vec3& vec) const -> std::size_t {
    std::size_t seed = 0;
    std::hash<float> hasher;
    hash_combine(seed, hasher(vec.x));
    hash_combine(seed, hasher(vec.y));
    hash_combine(seed, hasher(vec.z));
    return seed;
  }
};

template <>
struct hash<glm::ivec3> {
  auto operator()(const glm::ivec3& vec) const -> std::size_t {
    std::size_t seed = 0;
    std::hash<int> hasher;
    hash_combine(seed, hasher(vec.x));
    hash_combine(seed, hasher(vec.y));
    hash_combine(seed, hasher(vec.z));
    return seed;
  }
};

template <>
struct hash<glm::vec4> {
  auto operator()(const glm::vec4& vec) const -> std::size_t {
    std::size_t seed = 0;
    std::hash<float> hasher;
    hash_combine(seed, hasher(vec.x));
    hash_combine(seed, hasher(vec.y));
    hash_combine(seed, hasher(vec.z));
    hash_combine(seed, hasher(vec.w));
    return seed;
  }
};

template <>
struct hash<glm::u8vec4> {
  auto operator()(const glm::u8vec4& vec) const -> std::size_t {
    std::size_t seed = 0;
    std::hash<uint8_t> hasher;
    hash_combine(seed, hasher(vec.r));
    hash_combine(seed, hasher(vec.g));
    hash_combine(seed, hasher(vec.b));
    hash_combine(seed, hasher(vec.a));
    return seed;
  }
};
}
