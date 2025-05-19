#pragma once

namespace tr {

class TrackerManager {
public:
  TrackerManager() = default;
  ~TrackerManager() = default;

  TrackerManager(const TrackerManager&) = default;
  TrackerManager(TrackerManager&&) = delete;
  auto operator=(const TrackerManager&) -> TrackerManager& = default;
  auto operator=(TrackerManager&&) -> TrackerManager& = delete;
};

}
