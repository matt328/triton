#pragma once

#include "GlmCereal.hpp"
#include "cm/EntitySystemTypes.hpp"

namespace ed {

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

struct Orientation {
  glm::vec3 position{glm::zero<glm::vec3>()};
  glm::vec3 rotation{glm::zero<glm::vec3>()};

  template <class T>
  void serialize(T& archive) {
    archive(position, rotation);
  }
};

struct EntityData {
  std::string name;
  Orientation orientation;

  std::string modelName;
  std::string skeleton;
  std::vector<std::string> animations;

  glm::vec3 camPosition{glm::zero<glm::vec3>()};
  float yaw{0.f};
  float pitch{0.f};

  template <class T>
  void serialize(T& archive) {
    archive(name, orientation, modelName, skeleton, animations, camPosition, yaw, pitch);
  }
};

struct ChunkData {
  tr::EntityType entityId;
  glm::ivec3 location;
};

struct TerrainData {
  std::string name;
  glm::vec3 terrainSize;

  std::vector<ChunkData> chunkData;
  tr::EntityType entityId;

  template <class T>
  void serialize(T& archive) {
    archive(name, terrainSize);
  }
};

struct DataStore {
  // Assets
  std::unordered_map<std::string, SkeletonData> skeletons;
  std::unordered_map<std::string, AnimationData> animations;
  std::unordered_map<std::string, ModelData> models;

  // Scene
  std::unordered_map<std::string, EntityData> scene;

  // Terrain
  std::unordered_map<std::string, TerrainData> terrainMap;

  std::unordered_map<std::string, tr::EntityType> entityNameMap;

  template <class T>
  void serialize(T& archive) {
    archive(skeletons, animations, models, scene);
  }
};

}
