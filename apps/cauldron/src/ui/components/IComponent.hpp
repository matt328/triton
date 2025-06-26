#pragma once

#include "api/fx/Events.hpp"
#include "api/fx/IEventQueue.hpp"
#include "api/gw/editordata/EditorState.hpp"

namespace tr {
class IEventQueue;
}

namespace ed {

class IComponent {
public:
  IComponent() = default;
  virtual ~IComponent() = default;

  IComponent(const IComponent&) = default;
  IComponent(IComponent&&) = delete;
  auto operator=(const IComponent&) -> IComponent& = default;
  auto operator=(IComponent&&) -> IComponent& = delete;

  virtual auto render(const tr::EditorState& uiState) -> void = 0;
  virtual auto bindInput() -> void = 0;

  auto emitEvents(tr::IEventQueue& eventQueue) -> void {
    for (auto& event : pendingEvents) {
      eventQueue.emit(std::move(event));
    }
    pendingEvents.clear();
  }

protected:
  std::vector<tr::EventVariant> pendingEvents;
};

}
