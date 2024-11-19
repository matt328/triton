#include "DataFacade.hpp"

#include "cm/EntitySystemTypes.hpp"
#include "TaskQueue.hpp"

#include "tr/IGameplaySystem.hpp"

namespace ed::data {

   namespace cm = ::tr::cm;

   DataFacade::DataFacade(std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem,
                          std::shared_ptr<TaskQueue> newTaskQueue)
       : gameplaySystem{std::move(newGameplaySystem)}, taskQueue{std::move(newTaskQueue)} {
      Log.trace("Creating DataFacade");
   }

   DataFacade::~DataFacade() {
      Log.trace("Destroying DataFacade");
   }

   void DataFacade::update() const {
      taskQueue->processCompleteTasks();
   }

   void DataFacade::clear() {
      dataStore.animations.clear();
      dataStore.models.clear();
      dataStore.skeletons.clear();
      dataStore.scene.clear();
   }

   void DataFacade::createAABB() {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<float> dis(-50.f, 50.f);

      for (int i = 0; i < 50; ++i) {
         const auto min = glm::vec3{dis(gen), dis(gen), dis(gen)};
         const auto max = min + glm::vec3{5.f, 1.f, 1.f};
         //[[maybe_unused]] auto entityId = gameplayFacade.createDebugAABB(min, max);
      }
   }

   void DataFacade::addSkeleton(std::string_view name, const std::filesystem::path& path) {
      dataStore.skeletons.insert({name.data(), SkeletonData{name.data(), path.string()}});
      unsaved = true;
   }

   void DataFacade::removeSkeleton([[maybe_unused]] std::string_view name) {
   }

   void DataFacade::addAnimation(std::string_view name, const std::filesystem::path& path) {
      dataStore.animations.insert({name.data(), AnimationData{name.data(), path.string()}});
      unsaved = true;
   }

   void DataFacade::removeAnimation([[maybe_unused]] std::string_view name) {
   }

   void DataFacade::addModel(std::string_view name, const std::filesystem::path& path) {
      dataStore.models.insert({name.data(), ModelData{name.data(), path.string()}});
      unsaved = true;
   }

   void DataFacade::removeModel([[maybe_unused]] std::string_view name) {
   }

   void DataFacade::createStaticModel(std::string_view entityName,
                                      std::string_view modelName) noexcept {
      dataStore.scene.insert({entityName.data(),
                              EntityData{.name = entityName.data(),
                                         .position = glm::vec3{0.F},
                                         .rotation = glm::identity<glm::quat>(),
                                         .modelName = modelName.data()}});
      unsaved = true;
      const auto modelFilename = dataStore.models.at(modelName.data()).filePath;

      const auto onComplete = [this, entityName](cm::EntityType entity) {
         const auto* const name = entityName.data();
         entityNameMap.insert({name, entity});
         engineBusy = false;
         Log.info("Finished creating entity: id: {0}, name: {1}",
                  static_cast<int64_t>(entity),
                  name);
      };

      engineBusy = true;

      const auto task = [this, &modelFilename]() {
         return gameplaySystem->createStaticModelEntity(modelFilename);
      };

      const auto result = taskQueue->enqueue(task, onComplete);
   }

   void DataFacade::createAnimatedModel(const EntityData& entityData) {
      dataStore.scene.insert({entityData.name, entityData});
      unsaved = true;

      const auto modelFilename = dataStore.models.at(entityData.modelName).filePath;
      const auto skeletonFilename = dataStore.skeletons.at(entityData.skeleton).filePath;
      const auto animationFilename = dataStore.animations.at(entityData.animations[0]).filePath;
      const auto entityName = entityData.name;
      std::function<void(cm::EntityType)> fn = [this, entityName](cm::EntityType entity) {
         ZoneNamedN(z, "Create Entity", true);
         entityNameMap.insert({entityName, entity});
         engineBusy = false;
         Log.info("Finished creating entity: id: {0}, name: {1}",
                  static_cast<long long>(entity),
                  entityName);
      };
      engineBusy = true;

      const auto animatedEntityData =
          tr::gp::AnimatedModelData{.modelFilename = modelFilename,
                                    .skeletonFilename = skeletonFilename,
                                    .animationFilename = animationFilename};

      const auto task = [this, animatedEntityData]() {
         return gameplaySystem->createAnimatedModelEntity(animatedEntityData);
      };

      auto result = taskQueue->enqueue(task, fn);
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

   void DataFacade::createTerrain([[maybe_unused]] std::string_view terrainName) {
      const auto task = [&]() { gameplaySystem->createTerrain(); };

      std::function<void()> onComplete = [this]() { engineBusy = false; };
      engineBusy = true;
      auto result = taskQueue->enqueue(task, onComplete);
   }

   void DataFacade::save(const std::filesystem::path& outputFile) {
      auto os = std::ofstream(outputFile, std::ios::binary);
      cereal::BinaryOutputArchive output(os);
      output(dataStore);
      Log.info("Wrote binary output file to {0}", outputFile.string());
      unsaved = false;
   }

   void DataFacade::load(const std::filesystem::path& inputFile) {
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
               createStaticModel(entityData.name, entityData.modelName);
            } else {
               createAnimatedModel(entityData);
            }
         }

         unsaved = false;
      } catch (const std::exception& ex) { Log.error(ex.what()); }
   }

   void DataFacade::setEntityPosition(std::string_view name, const glm::vec3& newPosition) {
      auto& [name2, position, rotation, modelName, skeleton, animations] =
          dataStore.scene.at(name.data());
      position = newPosition;
      unsaved = true;
      Log.debug("need to push entity position change into engine");
   }
}
