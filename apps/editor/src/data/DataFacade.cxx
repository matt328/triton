#include "DataFacade.hpp"

namespace ed::data {
   DataFacade::~DataFacade() {
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

   void DataFacade::createEntity([[maybe_unused]] const std::string_view& entityName) {
   }

   void DataFacade::addAnimationToEntity([[maybe_unused]] const std::string_view& entityName,
                                         [[maybe_unused]] const std::string_view& animationName) {
   }

   void DataFacade::setEntitySkeleton([[maybe_unused]] const std::string_view& entityName,
                                      [[maybe_unused]] std::string_view& skeletonName) {
   }

   void DataFacade::createTerrain([[maybe_unused]] const std::string_view& terrainName) {
   }

   void DataFacade::save(const std::filesystem::path& outputFile) {
      auto os = std::ofstream(outputFile, std::ios::binary);
      cereal::BinaryOutputArchive output(os);
      output(dataStore);
      Log::info << "Wrote binary output file to " << outputFile.string() << std::endl;
      unsaved = false;
   }

   void DataFacade::load(const std::filesystem::path& inputFile) {
      try {
         {
            auto is = std::ifstream(inputFile, std::ios::binary);
            cereal::BinaryInputArchive input(is);
            input(dataStore);
            unsaved = false;
         }
      } catch (const std::exception& ex) { Log::error << ex.what() << std::endl; }
   }
}
