#include "DataFacade.hpp"

#include "cm/EntitySystemTypes.hpp"
#include "TaskQueue.hpp"
#include "cm/sdf/VoxelMetrics.hpp"
#include <bitset>

namespace ed::data {

   DataFacade::DataFacade(tr::ctx::GameplayFacade& gameplayFacade)
       : gameplayFacade{gameplayFacade} {
      taskQueue = std::make_unique<tr::util::TaskQueue>(4);
   }

   DataFacade::~DataFacade() { // NOLINT(*-use-equals-default)
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
      gameplayFacade.createDebugAABB(glm::vec3{0.f, 0.f, 0.f}, glm::vec3{5.f, 5.f, 5.f});
   }

   void DataFacade::addSkeleton(const std::string_view& name, const std::filesystem::path& path) {
      dataStore.skeletons.insert({name.data(), SkeletonData{name.data(), path.string()}});
      unsaved = true;
   }

   void DataFacade::removeSkeleton([[maybe_unused]] const std::string_view& name) {
   }

   void DataFacade::addAnimation(const std::string_view& name, const std::filesystem::path& path) {
      dataStore.animations.insert({name.data(), AnimationData{name.data(), path.string()}});
      unsaved = true;
   }

   void DataFacade::removeAnimation([[maybe_unused]] const std::string_view& name) {
   }

   void DataFacade::addModel(const std::string_view& name, const std::filesystem::path& path) {
      dataStore.models.insert({name.data(), ModelData{name.data(), path.string()}});
      unsaved = true;
   }

   void DataFacade::removeModel([[maybe_unused]] const std::string_view& name) {
   }

   void DataFacade::createStaticModel(const std::string_view& entityName,
                                      const std::string_view& modelName) noexcept {
      dataStore.scene.insert({entityName.data(),
                              EntityData{.name = entityName.data(),
                                         .position = glm::vec3{0.F},
                                         .rotation = glm::identity<glm::quat>(),
                                         .modelName = modelName.data()}});
      unsaved = true;
      const auto modelFilename = dataStore.models.at(modelName.data()).filePath;

      const auto onComplete = [this, entityName](tr::cm::EntityType entity) {
         const auto* const name = entityName.data();
         entityNameMap.insert({name, entity});
         engineBusy = false;
         Log.info("Finished creating entity: id: {0}, name: {1}",
                  static_cast<int64_t>(entity),
                  name);
      };

      engineBusy = true;

      const auto task = [this, &modelFilename]() {
         return gameplayFacade.createStaticModelEntity(modelFilename);
      };

      const auto result = taskQueue->enqueue(task, onComplete);
   }

   void DataFacade::createAnimatedModel(const std::string_view& entityName,
                                        const std::string_view& modelName,
                                        const std::string_view& skeletonName,
                                        const std::string_view& animationName) {
      auto name = std::string{entityName.data()};
      auto entityData = EntityData{.name = name,
                                   .position = glm::vec3{0.f},
                                   .rotation = glm::identity<glm::quat>(),
                                   .modelName = modelName.data(),
                                   .skeleton = skeletonName.data()};
      entityData.animations = std::vector<std::string>{animationName.data()};
      dataStore.scene.insert({name, entityData});
      unsaved = true;

      const auto modelFilename = dataStore.models.at(modelName.data()).filePath;
      const auto skeletonFilename = dataStore.skeletons.at(skeletonName.data()).filePath;
      const auto animationFilename = dataStore.animations.at(animationName.data()).filePath;

      std::function<void(tr::cm::EntityType)> fn = [this, name](tr::cm::EntityType entity) {
         ZoneNamedN(z, "Create Entity", true);
         entityNameMap.insert({name, entity});
         engineBusy = false;
         Log.info("Finished creating entity: id: {0}, name: {1}",
                  static_cast<long long>(entity),
                  name);
      };
      engineBusy = true;

      const auto task = [this, modelFilename, skeletonFilename, animationFilename]() {
         return gameplayFacade.createAnimatedModelEntity(modelFilename,
                                                         skeletonFilename,
                                                         animationFilename);
      };
      auto result = taskQueue->enqueue(task, fn);
   }

   void DataFacade::addAnimationToEntity([[maybe_unused]] const std::string_view& entityName,
                                         [[maybe_unused]] const std::string_view& animationName) {
      auto& entityData = dataStore.scene.at(entityName.data());
      entityData.animations.emplace_back(animationName.data());
      unsaved = true;
   }

   void DataFacade::setEntitySkeleton([[maybe_unused]] const std::string_view& entityName,
                                      [[maybe_unused]] const std::string_view& skeletonName) {
      auto& entityData = dataStore.scene.at(entityName.data());
      entityData.skeleton = skeletonName.data();
      unsaved = true;
   }

   void DataFacade::createTerrain([[maybe_unused]] const std::string_view& terrainName) {
      auto task = gameplayFacade.getCreateTerrainFn();

      std::function<void(tr::cm::EntityType)> onComplete = [this](tr::cm::EntityType entity) {
         engineBusy = false;
         const auto& cellData = tr::cm::sdf::VoxelDebugger::getInstance().getActiveCubePositions();
         for (const auto& cell : cellData) {
            Log.debug("{0}", cell.toString());
         }
      };
      engineBusy = true;
      auto result = taskQueue->enqueue(task, onComplete, 16);
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
               createAnimatedModel(entityData.name,
                                   entityData.modelName,
                                   entityData.skeleton,
                                   entityData.animations[0]);
            }
         }

         unsaved = false;
      } catch (const std::exception& ex) { Log.error(ex.what()); }
   }

   void DataFacade::setEntityPosition(const std::string_view& name, const glm::vec3& newPosition) {
      auto& [name2, position, rotation, modelName, skeleton, animations] =
          dataStore.scene.at(name.data());
      position = newPosition;
      unsaved = true;
      Log.debug("need to push entity position change into engine");
   }
}
