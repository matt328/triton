#pragma once

#include "bk/Hash.hpp"

namespace tr {

struct ImageRequest {
  std::string logicalName;
  vk::Format format;
  vk::Extent2D extent;
  vk::ImageUsageFlags usageFlags;
  vk::ImageAspectFlags aspectFlags;
  uint32_t mipLevels = 1;
  uint32_t layers = 1;
  std::optional<std::string> debugName = std::nullopt;

  auto operator==(const ImageRequest& other) const -> bool {
    return logicalName == other.logicalName && format == other.format && extent == other.extent &&
           usageFlags == other.usageFlags && aspectFlags == other.aspectFlags &&
           mipLevels == other.mipLevels && layers == other.layers && debugName == other.debugName;
  }
};

struct ImageInstanceKey {
  uint8_t frameId;

  auto operator==(const ImageInstanceKey& other) const -> bool {
    return frameId == other.frameId;
  }
};

struct ImageKey {
  ImageRequest request;
  ImageInstanceKey instance;

  auto operator==(const ImageKey& other) const -> bool {
    return request == other.request && instance == other.instance;
  }
};

}

namespace std {
template <>
struct std::hash<tr::ImageRequest> {
  auto operator()(const tr::ImageRequest& req) const -> std::size_t {
    std::size_t h = std::hash<std::string>{}(req.logicalName);
    h ^= std::hash<int>{}(static_cast<int>(req.format)) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(req.extent.width) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(req.extent.height) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(static_cast<uint32_t>(req.usageFlags)) + 0x9e3779b9 + (h << 6) +
         (h >> 2);
    h ^= std::hash<uint32_t>{}(static_cast<uint32_t>(req.aspectFlags)) + 0x9e3779b9 + (h << 6) +
         (h >> 2);
    h ^= std::hash<uint32_t>{}(req.mipLevels) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(req.layers) + 0x9e3779b9 + (h << 6) + (h >> 2);

    if (req.debugName.has_value()) {
      h ^= std::hash<std::string>{}(*req.debugName) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }

    return h;
  }
};

template <>
struct hash<tr::ImageInstanceKey> {
  auto operator()(const tr::ImageInstanceKey& v) const -> std::size_t {
    std::size_t h = 0;
    hash_combine(h, v.frameId);
    return h;
  }
};

template <>
struct hash<tr::ImageKey> {
  auto operator()(const tr::ImageKey& k) const -> std::size_t {
    std::size_t h = 0;
    hash_combine(h, k.request);
    hash_combine(h, k.instance);
    return h;
  }
};

}
