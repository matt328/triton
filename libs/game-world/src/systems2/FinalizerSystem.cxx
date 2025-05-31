#include "FinalizerSystem.hpp"
#include "components/Renderable.hpp"
#include "components/Transform.hpp"

namespace tr {

auto FinalizerSystem::update(entt::registry& registry, SimState& simState) -> void {
  simState.clear();
  simState.timeStamp = std::chrono::steady_clock::now();

  Log.trace(
      "setting state, timestamp={}",
      std::chrono::duration_cast<std::chrono::milliseconds>(simState.timeStamp.time_since_epoch())
          .count());

  const auto view = registry.view<Renderable, Transform>();
  const auto size = view.size_hint();

  simState.ensureCapacity(size);
  size_t current = 0;

  for (const auto& [entity, renderable, transform] : view.each()) {
    simState.stateHandles.push_back(
        StateHandles{.geometryHandle = renderable.geometryHandles.front()});

    simState.positions.push_back({.position = transform.position});
    simState.rotations.push_back({.rotation = transform.rotation});
    simState.scales.push_back({.scale = transform.scale});
    simState.objectMetadata.push_back(
        GpuObjectData{.transformIndex = static_cast<uint32_t>(simState.positions.size()),
                      .rotationIndex = static_cast<uint32_t>(simState.rotations.size()),
                      .scaleIndex = static_cast<uint32_t>(simState.scales.size()),
                      .geometryRegionId = static_cast<uint32_t>(current),
                      .materialId = 0L,
                      .animationId = 0L});
    ++current;
  }
}

}
