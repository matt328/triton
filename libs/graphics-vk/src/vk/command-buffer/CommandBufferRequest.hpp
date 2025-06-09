#pragma once

#include "r3/ComponentIds.hpp"
#include "vk/command-buffer/QueueType.hpp"

namespace tr {

struct CommandBufferRequest {
  std::thread::id threadId;
  uint8_t frameId;
  PassId passId;
  QueueType queueType = QueueType::Graphics;

  auto operator==(const CommandBufferRequest& other) const -> bool {
    return threadId == other.threadId && frameId == other.frameId && passId == other.passId &&
           queueType == other.queueType;
  }
};

}

namespace std {
template <>
struct hash<tr::CommandBufferRequest> {
  auto operator()(const tr::CommandBufferRequest& req) const -> std::size_t {
    std::size_t h1 = std::hash<std::thread::id>{}(req.threadId);
    std::size_t h2 = std::hash<uint8_t>{}(req.frameId);
    std::size_t h3 = std::hash<tr::PassId>{}(req.passId);
    std::size_t h4 = std::hash<uint8_t>{}(static_cast<uint8_t>(req.queueType));
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3); // basic mixing
  }
};
}
