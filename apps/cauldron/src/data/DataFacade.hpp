#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/TaskQueue.hpp"
#include "gp/components/Transform.hpp"
#include "tr/IGameplaySystem.hpp"
#include "DataStore.hpp"

/*
   Think of the editor as a tool for editing the project data nothing more.
   Whatever the rendering engine decides to do with that data, including it's internal state, is
   irrelevant to the editor. Editor just makes changes to its data model, and the rendering engine
   renders that data.

   In the case of scripts, the editor would alter the data such that a script is attached to an
   entity and that's it. The engine would sample the state of the data and evaluate the script at a
   certain timestep and render the data at that state.

   The data in a project represents all the data as it exists at t=0.

   Will need a way to start, stop, and reset the 'time' of the game from the editor.
*/

namespace tr {
class IGameplaySystem;
}

namespace ed {

class DataFacade {
public:
  DataFacade(std::shared_ptr<tr::IGameplaySystem> newGameplaySystem,
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

  void createTerrain(std::string_view terrainName);
  void createAABB();

  void save(const std::filesystem::path& outputFile);
  void load(const std::filesystem::path& inputFile);

  void entityTransformUpdated(std::string_view name, const tr::Transform& transform);

  auto getEntityNames() -> std::vector<std::tuple<std::string, tr::EntityType>>;

  [[nodiscard]] auto getEntityId(std::string_view name) const -> tr::EntityType {
    return dataStore.entityNameMap.at(name.data());
  }

  [[nodiscard]] auto getEntityData(std::string_view name) -> EntityData& {
    return dataStore.scene.at(name.data());
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
  std::shared_ptr<tr::IGameplaySystem> gameplaySystem;
  std::shared_ptr<tr::TaskQueue> taskQueue;

  bool unsaved{};
  bool engineBusy{};
  DataStore dataStore;
};

} // namespace ed::data
