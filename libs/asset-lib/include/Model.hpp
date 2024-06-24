#pragma once

#include "GlmCereal.hpp"

struct Vertex {
   glm::vec3 position;
   glm::vec3 normal;
   glm::vec2 uv;
   glm::vec4 color;
   glm::u8vec4 joints;
   glm::vec4 weights;
   glm::vec4 tangent;

   bool operator==(const Vertex& other) const {
      return position == other.position && normal == other.normal && uv == other.uv &&
             joints == other.joints && weights == other.weights;
   }

   template <class Archive>
   void serialize(Archive& archive) {
      archive(position, normal, uv, color, joints, weights, tangent);
   }
};

struct Model {
   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   std::unordered_map<int, int> jointRemaps;
   std::vector<glm::mat4> inverseBindPoses;

   [[nodiscard]] auto skinned() const {
      return !inverseBindPoses.empty();
   }

   [[nodiscard]] auto numJoints() const {
      return static_cast<int>(inverseBindPoses.size());
   }

   [[nodiscard]] auto highestJointIndex() const {
      return 0; // TODO
   }

   template <class Archive>
   void serialize(Archive& archive) {
      // archive(vertices, indices);
   }
};
