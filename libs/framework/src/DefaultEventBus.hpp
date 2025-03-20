#pragma once

#include "fx/Events.hpp"
#include "fx/IEventBus.hpp"

namespace tr {

class DefaultEventBus : public IEventBus {
public:
  DefaultEventBus();

  void subscribe(std::type_index type, std::function<void(const EventVariant&)> listener) override {
    listenersMap[type].push_back(listener);

    if (auto it = historyMap.find(type); it != historyMap.end()) {
      listener(it->second);
    }
  }

  void emit(std::type_index type, const EventVariant& event) override {
    historyMap[type] = event;
    if (auto it = listenersMap.find(type); it != listenersMap.end()) {
      for (auto& listener : it->second) {
        listener(event);
      }
    }
  }

private:
  std::unordered_map<std::type_index, std::vector<std::function<void(const EventVariant&)>>>
      listenersMap;
  std::unordered_map<std::type_index, EventVariant> historyMap;
};
}
