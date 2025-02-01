#pragma once

#include "cm/RenderData.hpp"
#include "AnimatedModelData.hpp"
#include "gp/components/Transform.hpp"

namespace tr {

using RenderDataTransferHandler = std::function<void(RenderData&)>;

class IGameplaySystem {
public:
  explicit IGameplaySystem() = default;
  virtual ~IGameplaySystem() = default;

  IGameplaySystem(const IGameplaySystem&) = default;
  IGameplaySystem(IGameplaySystem&&) = delete;
  auto operator=(const IGameplaySystem&) -> IGameplaySystem& = default;
  auto operator=(IGameplaySystem&&) -> IGameplaySystem& = delete;

  virtual void update() = 0;
  virtual void fixedUpdate() = 0;

  virtual void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) = 0;

  virtual auto createStaticModelEntity(std::string filename,
                                       std::string_view entityName,
                                       std::optional<Transform> initialTransform) -> void = 0;
  virtual auto createAnimatedModelEntity(const AnimatedModelData& modelData,
                                         std::optional<Transform> initialTransform) -> void = 0;
  virtual auto createTerrain() -> void = 0;
  virtual auto createDefaultCamera() -> void = 0;
  virtual auto createTestEntity(std::string_view name) -> void = 0;
  [[nodiscard]] virtual auto getRegistry() const -> std::shared_ptr<entt::registry> = 0;
};
}
