#pragma once

#include "tr/IEventBus.hpp"

namespace tr {

   class DefaultEventBus : public IEventBus {
    public:
      DefaultEventBus();

      void subscribe(std::type_index type,
                     std::function<void(const EventVariant&)> listener) override {
         listenersMap[type].push_back(listener);
      }

      void emit(std::type_index type, const EventVariant& event) override {
         auto it = listenersMap.find(type);
         if (it != listenersMap.end()) {
            for (auto& listener : it->second) {
               listener(event); // Call each listener with the event
            }
         }
      }

    private:
      std::unordered_map<std::type_index, std::vector<std::function<void(const EventVariant&)>>>
          listenersMap;
   };
}
