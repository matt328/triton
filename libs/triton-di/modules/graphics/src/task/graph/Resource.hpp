#pragma once

namespace tr {

constexpr auto SeedMagic = 0x9e3779b9;
constexpr auto LShift = 6u;
constexpr auto RShift = 2u;

class Resource {
public:
  Resource() = default;
  ~Resource() = default;

  Resource(const Resource&) = default;
  Resource(Resource&&) = delete;
  auto operator=(const Resource&) -> Resource& = default;
  auto operator=(Resource&&) -> Resource& = delete;

  auto operator==(const Resource& other) const -> bool;

private:
  std::string id;
  bool read = false;
  bool write = false;
  vk::PipelineStageFlags stages;

  friend struct std::hash<Resource>;
};

}

namespace std {

template <>
struct hash<tr::Resource> {
  auto operator()(const tr::Resource& resource) const -> std::size_t {
    std::size_t seed = 0;

    seed ^= std::hash<std::string>{}(resource.id) + tr::SeedMagic + (seed << tr::LShift) +
            (seed >> tr::RShift);

    seed ^= std::hash<bool>{}(resource.read) + tr::SeedMagic + (seed << tr::LShift) +
            (seed >> tr::RShift);

    seed ^= std::hash<bool>{}(resource.write) + tr::SeedMagic + (seed << tr::LShift) +
            (seed >> tr::RShift);

    seed ^= std::hash<uint32_t>{}(static_cast<uint32_t>(resource.stages)) + tr::SeedMagic +
            (seed << tr::LShift) + (seed >> tr::RShift);

    return seed;
  }
};

}
