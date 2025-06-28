#pragma once

#include "api/gw/editordata/GameObjectData.hpp"
#include "bk/Rando.hpp"

namespace tr {
class IEventQueue;
struct AddStaticModel;
struct StaticModelUploaded;
}

namespace ed {

class ApplicationController {
public:
  ApplicationController(std::shared_ptr<tr::IEventQueue> newEventQueue);
  ~ApplicationController() = default;

  ApplicationController(const ApplicationController&) = delete;
  ApplicationController(ApplicationController&&) = delete;
  auto operator=(const ApplicationController&) -> ApplicationController& = delete;
  auto operator=(ApplicationController&&) -> ApplicationController& = delete;

private:
  static inline const std::string UIGroup = "ui_group";

  std::shared_ptr<tr::IEventQueue> eventQueue;

  tr::MapKey requestIdGenerator;
  tr::MapKey batchIdGenerator;

  std::unordered_map<uint64_t, tr::GameObjectData> inFlightMap;

  auto handleAddStaticModel(const tr::AddStaticModel& event) -> void;
  auto handleStaticModelUploaded(const tr::StaticModelUploaded& event) -> void;
};

}
