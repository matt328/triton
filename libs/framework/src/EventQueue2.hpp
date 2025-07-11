#pragma once

#include "api/fx/IEventQueue.hpp"

namespace tr {

struct ThreadLocalData {
  std::mutex mutex;
  std::unordered_map<
      std::type_index,
      std::unordered_map<std::string, std::vector<std::function<void(std::shared_ptr<void>)>>>>
      handlers;
  std::deque<std::pair<std::type_index, std::shared_ptr<void>>> dispatchQueue;
};

class EventQueue2 : public IEventQueue {
public:
  void emitErased(std::type_index type,
                  std::shared_ptr<void> payload,
                  const std::string& channel) override {
    std::lock_guard lock(globalMutex);
    eventQueues[channel].emplace_back(type, std::move(payload));
  }

  void subscribeErased(std::type_index type,
                       std::function<void(std::shared_ptr<void>)> listener,
                       const std::string& channel) override {
    auto& data = getThreadLocal();
    std::lock_guard lock(data.mutex);
    data.handlers[type][channel].emplace_back(std::move(listener));
  }

  auto dispatchPending() -> void override {
    auto& data = getThreadLocal();
    {
      std::lock_guard lock(globalMutex);
      for (auto& [channel, queue] : eventQueues) {
        auto it = queue.begin();
        while (it != queue.end()) {
          auto& [type, payload] = *it;
          if (data.handlers.contains(type)) {
            data.dispatchQueue.emplace_back(type, std::move(payload));
            it = queue.erase(it);
          } else {
            ++it;
          }
        }
      }
    }

    for (auto& [type, payload] : data.dispatchQueue) {
      auto hit = data.handlers.find(type);
      if (hit != data.handlers.end()) {
        for (auto& [channel, listeners] : hit->second) {
          for (auto& fn : listeners) {
            fn(payload);
          }
        }
      }
    }
    data.dispatchQueue.clear();
  }

private:
  static auto getThreadLocal() -> ThreadLocalData& {
    thread_local ThreadLocalData data;
    return data;
  }

  std::mutex globalMutex;
  std::unordered_map<std::string, std::deque<std::pair<std::type_index, std::shared_ptr<void>>>>
      eventQueues;
};

}
