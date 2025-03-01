#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/RenderData.hpp"
#include "gp/AssetManager.hpp"
#include "gp/components/Animation.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/EditorInfo.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/Resources.hpp"
#include "gp/components/Transform.hpp"

namespace tr {

using CamFn = std::function<void(RenderData& renderData, const Camera&)>;

using StaticUpdateFn = std::function<void(RenderData& renderData,
                                          bool isTerrain,
                                          entt::entity,
                                          const Renderable&,
                                          const Transform&)>;

using DynamicUpdateFn = std::function<void(RenderData& renderData,
                                           entt::entity,
                                           const Animation&,
                                           const Renderable&,
                                           const Transform&)>;

using EntityCreatedFn = std::function<void(entt::registry&, entt::entity)>;

class EntityService {
public:
  EntityService(std::shared_ptr<AssetManager> newAssetManager);
  ~EntityService();

  EntityService(const EntityService&) = delete;
  EntityService(EntityService&&) = delete;
  auto operator=(const EntityService&) -> EntityService& = delete;
  auto operator=(EntityService&&) -> EntityService& = delete;

  auto updateAnimations(const std::function<void(entt::entity, Animation&)>& fn) -> void;

  auto updateCameraActions(const std::function<void(entt::entity, Camera&)>& fn) -> void;

  auto updateCameras(
      const std::function<void(uint32_t width, uint32_t height, entt::entity, Camera&)>& fn)
      -> void;

  auto updateTransforms(const std::function<void(entt::entity, Transform&)>& fn) -> void;

  auto updateRenderDataCamera(const CamFn& camUpdateFn,
                              const StaticUpdateFn& staticFn,
                              const DynamicUpdateFn& dynamicFn,
                              RenderData& renderData) -> void;

  auto registerEntityCreated(const EntityCreatedFn& fn) -> entt::connection;

  auto updateWindowDimensions(WindowDimensions windowDimensions) -> void;

  auto createStaticEntity(std::vector<MeshData> meshData,
                          Transform transform,
                          std::string_view name) -> tr::EntityType;

  auto createDynamicEntity(ModelData modelData,
                           Transform transform,
                           std::string_view name) -> tr::EntityType;

  auto createCamera(CameraInfo cameraInfo, std::string_view name, bool setDefault = true) -> void;

  auto removeEntity(EntityType entity) -> void;

  auto forEachEditorInfo(
      const std::function<void(tr::EntityType entity, EditorInfo& editorInfo)>& fn) -> void;

  auto setTransform(tr::EntityType entityId, Transform transform) -> void;

private:
  std::shared_ptr<AssetManager> assetManager;

  std::unique_ptr<entt::registry> registry;
  mutable TracySharedLockableN(std::shared_mutex, registryMutex, "EntityService");

  EntityCreatedFn entityCreatedFn;

  auto entityCreated([[maybe_unused]] entt::registry& reg,
                     [[maybe_unused]] entt::entity entity) const -> void;
};

}
