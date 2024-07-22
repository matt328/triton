#include "DataFacade.hpp"

#include "FutureMonitor.hpp"
#include "cm/EntitySystemTypes.hpp"

namespace ed::data {

   DataFacade::DataFacade(tr::ctx::GameplayFacade& gameplayFacade)
       : gameplayFacade{gameplayFacade} {
      futureMonitor = std::make_unique<FutureMonitor>();
   }

   DataFacade::~DataFacade() {
   }

   void DataFacade::update() {
      futureMonitor->update();
   }

   void DataFacade::clear() {
      dataStore.animations.clear();
      dataStore.models.clear();
      dataStore.skeletons.clear();
      dataStore.scene.clear();
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
                                      const std::string_view& modelName) {
      dataStore.scene.insert({entityName.data(),
                              EntityData{.name = entityName.data(),
                                         .position = glm::vec3{0.f},
                                         .rotation = glm::identity<glm::quat>(),
                                         .modelName = modelName.data()}});
      unsaved = true;
      const auto modelFilename = dataStore.models.at(modelName.data()).filePath;

      std::function<void(tr::cm::EntityType)> fn = [this, entityName](tr::cm::EntityType entity) {
         const auto name = entityName.data();
         entityNameMap.insert({name, entity});
         engineBusy = false;
         Log.info("Finished creating entity: id: {0}, name: {1}",
                  static_cast<long long>(entity),
                  name);
      };

      engineBusy = true;

      futureMonitor->monitorFuture(gameplayFacade.createStaticModelEntity(modelFilename), fn);
   }

   void DataFacade::createAnimatedModel(const std::string_view& entityName,
                                        const std::string_view& modelName,
                                        const std::string_view& skeletonName,
                                        const std::string_view& animationName) {
      auto entityData = EntityData{.name = entityName.data(),
                                   .position = glm::vec3{0.f},
                                   .rotation = glm::identity<glm::quat>(),
                                   .modelName = modelName.data(),
                                   .skeleton = skeletonName.data()};
      entityData.animations = std::vector<std::string>{animationName.data()};
      dataStore.scene.insert({entityName.data(), entityData});
      unsaved = true;

      const auto modelFilename = dataStore.models.at(modelName.data()).filePath;
      const auto skeletonFilename = dataStore.skeletons.at(skeletonName.data()).filePath;
      const auto animationFilename = dataStore.animations.at(animationName.data()).filePath;

      std::function<void(tr::cm::EntityType)> fn = [this, entityName](tr::cm::EntityType entity) {
         const auto name = entityName.data();
         entityNameMap.insert({name, entity});
         engineBusy = false;
         Log.info("Finished creating entity: id: {0}, name: {1}",
                  static_cast<long long>(entity),
                  name);
      };
      engineBusy = true;

      futureMonitor->monitorFuture(gameplayFacade.createAnimatedModelEntity(modelFilename,
                                                                            skeletonFilename,
                                                                            animationFilename),
                                   fn);
   }

   void DataFacade::addAnimationToEntity([[maybe_unused]] const std::string_view& entityName,
                                         [[maybe_unused]] const std::string_view& animationName) {
      auto& entityData = dataStore.scene.at(entityName.data());
      entityData.animations.emplace_back(animationName.data());
      unsaved = true;
   }

   void DataFacade::setEntitySkeleton([[maybe_unused]] const std::string_view& entityName,
                                      [[maybe_unused]] std::string_view& skeletonName) {
      auto& entityData = dataStore.scene.at(entityName.data());
      entityData.skeleton = skeletonName.data();
      unsaved = true;
   }

   void DataFacade::createTerrain([[maybe_unused]] const std::string_view& terrainName) {
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
         input(dataStore);
         unsaved = false;
      } catch (const std::exception& ex) { Log.error(ex.what()); }
   }

   void DataFacade::setEntityPosition(const std::string_view& name, glm::vec3 newPosition) {
      auto& entityData = dataStore.scene.at(name.data());
      entityData.position = newPosition;
      unsaved = true;
      Log.debug("need to push entity position change into engine");
   }
}
