#pragma once

#include "GlmCereal.hpp"

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
      std::string modelName;
      std::string skeleton;
      std::vector<std::string> animations;
      glm::vec3 position;
      glm::quat rotation;

      template <class T>
      void serialize(T& archive) {
         archive(name, modelName, skeleton, animations, position, rotation);
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

   class DataFacade {
    public:
      DataFacade() = default;
      ~DataFacade();

      DataFacade(const DataFacade&) = default;
      DataFacade& operator=(const DataFacade&) = default;

      DataFacade(DataFacade&&) = delete;
      DataFacade& operator=(DataFacade&&) = delete;

      void clear();

      void addSkeleton(const std::string_view& name, const std::filesystem::path& path);
      void removeSkeleton(const std::string_view& name);

      void addAnimation(const std::string_view& name, const std::filesystem::path& path);
      void removeAnimation(const std::string_view& name);

      void addModel(const std::string_view& name, const std::filesystem::path& path);
      void removeModel(const std::string_view& name);

      void createEntity(const std::string_view& entityName);

      void addAnimationToEntity(const std::string_view& entityName,
                                const std::string_view& animationName);

      void setEntitySkeleton(const std::string_view& entityName, std::string_view& skeletonName);

      void createTerrain(const std::string_view& terrainName);

      void save(const std::filesystem::path& outputFile);
      void load(const std::filesystem::path& inputFile);

      [[nodiscard]] auto isUnsaved() const {
         return unsaved;
      }

    private:
      bool unsaved{};
      DataStore dataStore;
   };
}