#include "DataFacade.hpp"

#include "cm/TaskQueue.hpp"

#include "tr/IGameplaySystem.hpp"

namespace ed {

DataFacade::DataFacade(std::shared_ptr<tr::IGameplaySystem> newGameplaySystem,
                       std::shared_ptr<tr::TaskQueue> newTaskQueue)
    : gameplaySystem{std::move(newGameplaySystem)}, taskQueue{std::move(newTaskQueue)} {
  Log.trace("Creating DataFacade");
}

DataFacade::~DataFacade() {
  Log.trace("Destroying DataFacade");
}

void DataFacade::update() const {
}

void DataFacade::clear() {
  dataStore.animations.clear();
  dataStore.models.clear();
  dataStore.skeletons.clear();
  dataStore.scene.clear();
}

void DataFacade::createAABB() {
  // std::random_device rd;
  // std::mt19937 gen(rd());
  // std::uniform_real_distribution<float> dis(-50.f, 50.f);

  // for (int i = 0; i < 50; ++i) {
  //    const auto min = glm::vec3{dis(gen), dis(gen), dis(gen)};
  //    const auto max = min + glm::vec3{5.f, 1.f, 1.f};
  //    [[maybe_unused]] auto entityId = gameplayFacade.createDebugAABB(min, max);
  // }
}

void DataFacade::addSkeleton(std::string_view name, const std::filesystem::path& path) {
  dataStore.skeletons.insert(
      {name.data(), SkeletonData{.name = name.data(), .filePath = path.string()}});
  unsaved = true;
}

void DataFacade::removeSkeleton([[maybe_unused]] std::string_view name) {
}

void DataFacade::addAnimation(std::string_view name, const std::filesystem::path& path) {
  dataStore.animations.insert(
      {name.data(), AnimationData{.name = name.data(), .filePath = path.string()}});
  unsaved = true;
}

void DataFacade::removeAnimation([[maybe_unused]] std::string_view name) {
}

void DataFacade::addModel(std::string_view name, const std::filesystem::path& path) {
  dataStore.models.insert({name.data(), ModelData{.name = name.data(), .filePath = path.string()}});
  unsaved = true;
}

void DataFacade::removeModel([[maybe_unused]] std::string_view name) {
}

void DataFacade::createStaticModel(const EntityData& entityData) noexcept {
  unsaved = true;
  const auto modelFilename = dataStore.models.at(entityData.modelName).filePath;
  const auto entityName = entityData.name;

  const auto onComplete = [this, entityName, entityData](tr::EntityType entityId) {
    dataStore.scene.insert({entityData.name, entityData});
    dataStore.entityNameMap.insert({entityName, entityId});
    engineBusy = false;
    Log.info("Finished creating entity: name: {}", entityData.name);
  };

  engineBusy = true;

  const auto transform = tr::Transform{.rotation = entityData.orientation.rotation,
                                       .position = entityData.orientation.position};

  const auto task = [this, modelFilename, entityName, transform] {
    return gameplaySystem->createStaticModelEntity(modelFilename,
                                                   entityName,
                                                   std::make_optional(transform));
  };

  taskQueue->enqueue(task, onComplete);
}

void DataFacade::createAnimatedModel(const EntityData& entityData) {
  unsaved = true;

  const auto modelFilename = dataStore.models.at(entityData.modelName).filePath;
  const auto skeletonFilename = dataStore.skeletons.at(entityData.skeleton).filePath;
  const auto animationFilename = dataStore.animations.at(entityData.animations[0]).filePath;
  const auto entityName = entityData.name;

  const auto onComplete = [this, entityName, entityData](tr::EntityType entityId) {
    dataStore.entityNameMap.insert({entityName, entityId});
    dataStore.scene.insert({entityName, entityData});
    engineBusy = false;
  };
  engineBusy = true;

  const auto animatedEntityData = tr::AnimatedModelData{.modelFilename = modelFilename,
                                                        .skeletonFilename = skeletonFilename,
                                                        .animationFilename = animationFilename,
                                                        .entityName = entityName};

  const auto transform = tr::Transform{.rotation = entityData.orientation.rotation,
                                       .position = entityData.orientation.position};

  const auto task = [this, animatedEntityData, transform] {
    return gameplaySystem->createAnimatedModelEntity(animatedEntityData,
                                                     std::make_optional(transform));
  };

  taskQueue->enqueue(task, onComplete);
}

auto DataFacade::deleteEntity(std::string_view name) noexcept -> void {
  dataStore.scene.erase(name.data());
  dataStore.entityNameMap.erase(name.data());
}

void DataFacade::addAnimationToEntity([[maybe_unused]] std::string_view entityName,
                                      [[maybe_unused]] std::string_view animationName) {
  auto& entityData = dataStore.scene.at(entityName.data());
  entityData.animations.emplace_back(animationName.data());
  unsaved = true;
}

void DataFacade::setEntitySkeleton([[maybe_unused]] std::string_view entityName,
                                   [[maybe_unused]] std::string_view skeletonName) {
  auto& entityData = dataStore.scene.at(entityName.data());
  entityData.skeleton = skeletonName.data();
  unsaved = true;
}

void DataFacade::createTerrain(std::string_view terrainName, glm::vec3 terrainSize) {

  const auto task = [this, &terrainName, terrainSize] {
    return gameplaySystem->createTerrain(terrainName, terrainSize);
  };

  const auto onComplete = [this, terrainName, terrainSize](const tr::TerrainResult& result) {
    dataStore.entityNameMap.emplace(terrainName, result.definitionId);
    dataStore.terrainMap.emplace(
        terrainName,
        TerrainData{.name = std::string{terrainName},
                    .terrainSize = terrainSize,
                    .chunkIds = std::ranges::to<std::vector>(std::views::keys(result.chunkData))});

    for (const auto& [id, name] : result.chunkData) {
      dataStore.entityNameMap.emplace(name, id);
    }

    engineBusy = false;
  };

  engineBusy = true;
  taskQueue->enqueue(task, onComplete);
}

void DataFacade::save(const std::filesystem::path& outputFile) {
  auto os = std::ofstream(outputFile, std::ios::binary);
  cereal::BinaryOutputArchive output(os);
  output(dataStore);
  Log.info("Wrote binary output file to {0}", outputFile.string());
  unsaved = false;
}

void DataFacade::entityTransformUpdated(std::string_view name, const tr::Transform& transform) {
  auto& entityData = dataStore.scene.at(name.data());
  entityData.orientation.position = transform.position;
  entityData.orientation.rotation = transform.rotation;
  unsaved = true;
}

void DataFacade::load(const std::filesystem::path& inputFile) {
  Log.debug("Loading Project File");
  try {
    auto is = std::ifstream(inputFile, std::ios::binary);
    cereal::BinaryInputArchive input(is);
    auto tempStore = DataStore{};
    input(tempStore);

    clear();

    for (const auto& skeleton : tempStore.skeletons | std::views::values) {
      addSkeleton(skeleton.name, skeleton.filePath);
    }

    for (const auto& animation : tempStore.animations | std::views::values) {
      addAnimation(animation.name, animation.filePath);
    }

    for (const auto& model : tempStore.models | std::views::values) {
      addModel(model.name, model.filePath);
    }

    for (const auto& entityData : tempStore.scene | std::views::values) {
      if (entityData.animations.empty()) {
        createStaticModel(entityData);
      } else {
        createAnimatedModel(entityData);
      }
    }

    unsaved = false;
  } catch (const std::exception& ex) { Log.error(ex.what()); }
}

auto DataFacade::getEntityNames() -> std::vector<std::tuple<std::string, tr::EntityType>> {
  auto names = std::vector<std::tuple<std::string, tr::EntityType>>{};
  for (const auto& [name, _] : dataStore.scene) {
    names.emplace_back(name, dataStore.entityNameMap.at(name));
  }

  for (const auto& [name, _] : dataStore.terrainMap) {
    names.emplace_back(name, dataStore.entityNameMap.at(name));
  }

  return names;
}
}
