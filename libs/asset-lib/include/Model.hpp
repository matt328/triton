#pragma once

#include "GlmCereal.hpp"

struct Vertex {
   glm::vec3 position;
   glm::vec3 normal;
   glm::vec2 uv;
   glm::uvec4 joints;
   glm::vec4 weights;

   bool operator==(const Vertex& other) const {
      return position == other.position && normal == other.normal && uv == other.uv &&
             joints == other.joints && weights == other.weights;
   }

   template <class Archive>
   void serialize(Archive& archive) {
      archive(position, normal, uv, joints, weights);
   }
};

struct Model {
   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   /// This maps the heirarchical order of joints back to the order the joints vertex attribute
   /// matches. First is the hierarchical order, second is the vertex attribute order.
   /*
      Note: given a list of matrices from the sampling job, for each one in order, get its index,
      then get the matching vertex attribute order number from the map, and place the matrix in a
      vector at that position to be passed to the shader
   */
   std::unordered_map<uint32_t, uint32_t> jointMap;

   template <class Archive>
   void serialize(Archive& archive) {
      archive(vertices, indices);
   }
};
