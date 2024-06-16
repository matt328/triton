#pragma once

#include <cereal/types/vector.hpp>

namespace glm {

   template <class Archive>
   void serialize(Archive& archive, glm::vec2& v) {
      archive(v.x, v.y);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::vec3& v) {
      archive(v.x, v.y, v.z);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::vec4& v) {
      archive(v.x, v.y, v.z, v.w);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::ivec2& v) {
      archive(v.x, v.y);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::ivec3& v) {
      archive(v.x, v.y, v.z);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::ivec4& v) {
      archive(v.x, v.y, v.z, v.w);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::uvec2& v) {
      archive(v.x, v.y);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::uvec3& v) {
      archive(v.x, v.y, v.z);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::uvec4& v) {
      archive(v.x, v.y, v.z, v.w);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::dvec2& v) {
      archive(v.x, v.y);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::dvec3& v) {
      archive(v.x, v.y, v.z);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::dvec4& v) {
      archive(v.x, v.y, v.z, v.w);
   }

   // glm matrices serialization
   template <class Archive>
   void serialize(Archive& archive, glm::mat2& m) {
      archive(m[0], m[1]);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::dmat2& m) {
      archive(m[0], m[1]);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::mat3& m) {
      archive(m[0], m[1], m[2]);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::mat4& m) {
      archive(m[0], m[1], m[2], m[3]);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::dmat4& m) {
      archive(m[0], m[1], m[2], m[3]);
   }

   template <class Archive>
   void serialize(Archive& archive, glm::quat& q) {
      archive(q.x, q.y, q.z, q.w);
   }
   template <class Archive>
   void serialize(Archive& archive, glm::dquat& q) {
      archive(q.x, q.y, q.z, q.w);
   }

}

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
