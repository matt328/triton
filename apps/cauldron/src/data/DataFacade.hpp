#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "DataStore.hpp"

namespace tr {
class IGameWorldSystem;
class TaskQueue;
}

namespace ed {

class DataFacade {
public:
  DataFacade(std::shared_ptr<tr::IGameWorldSystem> newGameWorldSystem,
             std::shared_ptr<tr::TaskQueue> newTaskQueue);
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

  auto getEntityNames() -> std::vector<std::tuple<std::string, tr::EntityType>>;

  [[nodiscard]] auto getEntityId(std::string_view name) const -> tr::EntityType {
    return dataStore.entityNameMap.at(name.data());
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
  std::shared_ptr<tr::IGameWorldSystem> gameWorldSystem;
  std::shared_ptr<tr::TaskQueue> taskQueue;

  bool unsaved{};
  bool engineBusy{};
  DataStore dataStore;
};

} // namespace ed::data
