#pragma once

#include "api/fx/ResourceEvents.hpp"
#include "api/gw/GameObjectType.hpp"
#include "DataStore.hpp"
#include "bk/Rando.hpp"

namespace tr {
class IGameWorldSystem;
class IEventQueue;
}

namespace ed {

class DataFacade {
public:
  explicit DataFacade(std::shared_ptr<tr::IEventQueue> newEventQueue);
  ~DataFacade();

  DataFacade(const DataFacade&) = delete;
  auto operator=(const DataFacade&) -> DataFacade& = delete;

  DataFacade(DataFacade&&) = delete;
  auto operator=(DataFacade&&) -> DataFacade& = delete;

  void update() const;

  void clear();

  void addSkeleton(std::string_view name, const std::filesystem::path& path);
  void removeSkeleton(std::string_view name);

  void addAnimation(std::string_view name, const std::filesystem::path& path);
  void removeAnimation(std::string_view name);

  void addModel(std::string_view name, const std::filesystem::path& path);
  void removeModel(std::string_view name);

  void createStaticModel(const EntityData& entityData) noexcept;

  void createAnimatedModel(const EntityData& entityData);

  auto deleteEntity(std::string_view entityName) noexcept -> void;

  void addAnimationToEntity(std::string_view entityName, std::string_view animationName);

  void setEntitySkeleton(std::string_view entityName, std::string_view skeletonName);

  void createTerrain(std::string_view terrainName, glm::vec3 terrainSize);
  void createAABB();

  void save(const std::filesystem::path& outputFile);
  void load(const std::filesystem::path& inputFile);

  auto getEntityNames() -> std::vector<std::tuple<std::string, tr::GameObjectId>>;

  [[nodiscard]] auto getEntityId(std::string_view name) const -> tr::GameObjectId {
    // return dataStore.entityNameMap.at(name.data());
    return static_cast<tr::GameObjectId>(0);
  }

  [[nodiscard]] auto getEntityData(std::string_view name) -> EntityData* {
    if (dataStore.scene.contains(name.data())) {
      return &dataStore.scene.at(name.data());
    }
    return nullptr;
  }

  [[nodiscard]] auto getTerrainData(std::string_view name) -> TerrainData* {
    if (dataStore.terrainMap.contains(name.data())) {
      return &dataStore.terrainMap.at(name.data());
    }
    return nullptr;
  }

  [[nodiscard]] auto isUnsaved() const {
    return unsaved;
  }

  [[nodiscard]] auto isEngineBusy() const {
    return engineBusy;
  }

  [[nodiscard]] auto getSkeletons() const -> const auto& {
    return dataStore.skeletons;
  }

  [[nodiscard]] auto getAnimations() const -> const auto& {
    return dataStore.animations;
  }

  [[nodiscard]] auto getModels() const -> const auto& {
    return dataStore.models;
  }

  [[nodiscard]] auto getScene() const -> const auto& {
    return dataStore.scene;
  }

private:
  std::shared_ptr<tr::IEventQueue> eventQueue;

  bool unsaved{};
  bool engineBusy{};
  DataStore dataStore;

  tr::MapKey requestIdGenerator;

  std::unordered_map<uint64_t, EntityData> inFlightMap;

  auto testResources() -> void;
  auto handleStaticModelResponse(const tr::StaticModelResponse& response) -> void;

  static auto scheduleDelayed(std::function<void()> func, std::chrono::milliseconds delay) -> void;
};

}
