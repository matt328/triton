#pragma once

#include "GlmCereal.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "tr/GameplayFacade.hpp"

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

namespace ed::data {

   struct SkeletonData {
      std::string name;
      std::string filePath;

      template <class T>
      void serialize(T& archive) {
         archive(name, filePath);
      }
   };

   struct AnimationData {
      std::string name;
      std::string filePath;

      template <class T>
      void serialize(T& archive) {
         archive(name, filePath);
      }
   };

   struct ModelData {
      std::string name;
      std::string filePath;

      template <class T>
      void serialize(T& archive) {
         archive(name, filePath);
      }
   };

   struct EntityData {
      std::string name;
      glm::vec3 position;
      glm::quat rotation;
      std::string modelName;
      std::string skeleton;
      std::vector<std::string> animations;

      template <class T>
      void serialize(T& archive) {
         archive(name, position, rotation, modelName, skeleton, animations);
      }
   };

   struct TerrainData {
      std::string name;
      // adjustable terrain params
      // Right now this is just a marker for the engine to do render whatever it thinks a terrain is
      template <class T>
      void serialize(T& archive) {
         archive(name);
      }
   };

   struct DataStore {
      // Assets
      std::unordered_map<std::string, SkeletonData> skeletons;
      std::unordered_map<std::string, AnimationData> animations;
      std::unordered_map<std::string, ModelData> models;

      // Scene
      std::unordered_map<std::string, EntityData> scene;

      template <class T>
      void serialize(T& archive) {
         archive(skeletons, animations, models, scene);
      }
   };

   class FutureMonitor;

   class DataFacade {
    public:
      DataFacade(tr::ctx::GameplayFacade& gameplayFacade);
      ~DataFacade();

      DataFacade(const DataFacade&) = delete;
      DataFacade& operator=(const DataFacade&) = delete;

      DataFacade(DataFacade&&) = delete;
      DataFacade& operator=(DataFacade&&) = delete;

      void update();

      void clear();

      void addSkeleton(const std::string_view& name, const std::filesystem::path& path);
      void removeSkeleton(const std::string_view& name);

      void addAnimation(const std::string_view& name, const std::filesystem::path& path);
      void removeAnimation(const std::string_view& name);

      void addModel(const std::string_view& name, const std::filesystem::path& path);
      void removeModel(const std::string_view& name);

      void createStaticModel(const std::string_view& entityName, const std::string_view& modelName);

      void addAnimationToEntity(const std::string_view& entityName,
                                const std::string_view& animationName);

      void setEntitySkeleton(const std::string_view& entityName, std::string_view& skeletonName);

      void createTerrain(const std::string_view& terrainName);

      void save(const std::filesystem::path& outputFile);
      void load(const std::filesystem::path& inputFile);

      void setEntityPosition(const std::string_view& name, glm::vec3 newPosition);

      [[nodiscard]] EntityData getEntityData(const std::string_view& name) const {
         return dataStore.scene.at(name.data());
      }

      [[nodiscard]] auto isUnsaved() const {
         return unsaved;
      }

      [[nodiscard]] const auto& getSkeletons() const {
         return dataStore.skeletons;
      }

      [[nodiscard]] const auto& getAnimations() const {
         return dataStore.animations;
      }

      [[nodiscard]] const auto& getModels() const {
         return dataStore.models;
      }

      [[nodiscard]] const auto& getScene() const {
         return dataStore.scene;
      }

    private:
      bool unsaved{};
      bool engineBusy{};
      DataStore dataStore;
      tr::ctx::GameplayFacade& gameplayFacade;
      std::unique_ptr<FutureMonitor> futureMonitor;
      std::unordered_map<std::string, tr::cm::EntityType> entityNameMap{};
   };
}
