#include "ufbx.h"
#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include "Model.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>

namespace std {
   template <>
   struct hash<Vertex> {
      std::size_t operator()(const Vertex& vertex) const {
         std::size_t h1 = std::hash<float>{}(vertex.position.x) ^
                          std::hash<float>{}(vertex.position.y) << 1 ^
                          std::hash<float>{}(vertex.position.z) << 2;
         std::size_t h2 = std::hash<float>{}(vertex.normal.x) ^
                          std::hash<float>{}(vertex.normal.y) << 1 ^
                          std::hash<float>{}(vertex.normal.z) << 2;
         std::size_t h3 = std::hash<float>{}(vertex.uv.x) ^ std::hash<float>{}(vertex.uv.y) << 1;
         std::size_t h4 = std::hash<unsigned int>{}(vertex.joints.x) ^
                          std::hash<unsigned int>{}(vertex.joints.y) << 1 ^
                          std::hash<unsigned int>{}(vertex.joints.z) << 2 ^
                          std::hash<unsigned int>{}(vertex.joints.w) << 3;
         std::size_t h5 =
             std::hash<float>{}(vertex.weights.x) ^ std::hash<float>{}(vertex.weights.y) << 1 ^
             std::hash<float>{}(vertex.weights.z) << 2 ^ std::hash<float>{}(vertex.weights.w) << 3;
         return h1 ^ h2 ^ h3 ^ h4 ^ h5;
      }
   };

   template <>
   struct equal_to<Vertex> {
      bool operator()(const Vertex& lhs, const Vertex& rhs) const {
         return lhs.position == rhs.position && lhs.normal == rhs.normal && lhs.uv == rhs.uv &&
                lhs.joints == rhs.joints && lhs.weights == rhs.weights;
      }
   };
}

int main() {

   Log::LogManager::getInstance().setMinLevel(Log::Level::Info);

   const auto filename = R"(C:\Users\Matt\Projects\game-assets\models\working\Walking.fbx)";

   const auto skeletonPath =
       std::filesystem::path{R"(C:\Users\Matt\Projects\game-assets\models\working\skeleton.ozz)"};

   auto loadOpts =
       ufbx_load_opts{.target_axes = ufbx_axes_right_handed_y_up, .target_unit_meters = 1.f};

   auto fbxError = ufbx_error{};

   auto scene = ufbx_load_file(filename, &loadOpts, &fbxError);

   if (!scene) {
      fprintf(stderr, "Failed to load scene: %s\n", fbxError.description.data);
      return 1;
   }

   auto& skin = scene->meshes[0]->skin_deformers[0];

   int i = 0;
   for (const auto& cluster : skin->clusters) {
      Log::debug << i << " cluster: " << cluster->bone_node->name.data << std::endl;
      ++i;
   }

   constexpr size_t MAX_WEIGHTS = 4;

   auto allVertices = std::vector<Vertex>{};

   auto uniqueVertices = std::unordered_map<Vertex, uint32_t>{};
   auto newVertices = std::vector<Vertex>{};
   auto newIndices = std::vector<uint32_t>{};

   for (const auto& mesh : scene->meshes) {
      for (const auto& face : mesh->faces) {
         for (uint32_t vertex = 0; vertex < face.num_indices; vertex++) {
            auto index = face.index_begin + vertex;

            auto position = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
            auto positionv = glm::vec3(position.x, position.y, position.z);

            auto mat4 = glm::identity<glm::mat4>();
            mat4 = glm::translate(mat4, positionv);

            auto normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, index);
            auto uv = ufbx_get_vertex_vec2(&mesh->vertex_uv, index);

            auto joints = std::vector<uint32_t>{};
            auto weights = std::vector<float>{};

            auto vertexIndex = mesh->vertex_indices.data[index];

            auto skinVertex = skin->vertices.data[vertexIndex];
            size_t numWeights = skinVertex.num_weights;
            if (numWeights > MAX_WEIGHTS) {
               numWeights = MAX_WEIGHTS;
            }

            auto totalWeight = 0.f;

            for (size_t i = 0; i < numWeights; ++i) {
               auto skinWeight = skin->weights.data[skinVertex.weight_begin + i];
               joints.push_back(skinWeight.cluster_index);
               weights.push_back((float)skinWeight.weight);
               totalWeight += (float(skinWeight.weight));
            }

            while (joints.size() < MAX_WEIGHTS) {
               joints.push_back(0.f);
            }

            while (weights.size() < MAX_WEIGHTS) {
               weights.push_back(0.f);
            }

            for (size_t i = 0; i < numWeights; i++) {
               weights[i] /= totalWeight;
            }

            const auto v =
                Vertex{.position = glm::vec3(position.x, position.y, position.z),
                       .normal = glm::vec3(normal.x, normal.y, normal.z),
                       .uv = glm::vec2(uv.x, uv.y),
                       .joints = glm::uvec4(joints[0], joints[1], joints[2], joints[3]),
                       .weights = glm::vec4(weights[0], weights[1], weights[2], weights[3])};

            allVertices.push_back(v);

            if (uniqueVertices.find(v) == uniqueVertices.end()) {
               uniqueVertices[v] = static_cast<uint32_t>(newVertices.size());
               newVertices.push_back(v);
            }
            newIndices.push_back(uniqueVertices[v]);
         }
      }
   }

   Log::debug << "allVertices.size(): " << allVertices.size() << std::endl;
   Log::debug << "newVertices.size(): " << newVertices.size() << std::endl;
   Log::debug << "newIndices.size(): " << newIndices.size() << std::endl;

   auto model = Model{.vertices = newVertices, .indices = newIndices};

   const auto threeVertices = std::vector<Vertex>(newVertices.begin(), newVertices.begin() + 3);
   const auto threeIndices = std::vector<uint32_t>(newIndices.begin(), newIndices.begin() + 3);
   const auto smallModel = Model{.vertices = threeVertices, .indices = threeIndices};

   auto ss = std::stringstream{};
   auto modelOut = Model{};

   const auto binFile = R"(C:\Users\Matt\Projects\game-assets\models\working\cereal.bin)";

   {
      auto os = std::ofstream(binFile, std::ios::binary);
      cereal::BinaryOutputArchive output(os);
      output(model);
   }

   {
      auto is = std::ifstream(binFile, std::ios::binary);
      cereal::BinaryInputArchive input(is);
      input(modelOut);
   }

   uint32_t minJointIndex = UINT32_MAX;
   uint32_t maxJointIndex = 0;
   for (const auto& vertex : allVertices) {
      for (int i = 0; i < 4; ++i) {
         minJointIndex = std::min(minJointIndex, vertex.joints[i]);
         maxJointIndex = std::max(maxJointIndex, vertex.joints[i]);
      }
   }

   Log::debug << "min joint index: " << minJointIndex << std::endl;
   Log::debug << "max joint index: " << maxJointIndex << std::endl;

   ozz::io::File file(skeletonPath.string().c_str(), "rb");

   if (!file.opened()) {
      Log::error << "Failed to open skeleton file " << filename << "." << std::endl;
   }

   ozz::io::IArchive archive(&file);
   if (!archive.TestTag<ozz::animation::Skeleton>()) {
      Log::error << "Failed to load skeleton instance from file " << filename << "." << std::endl;
   }

   // The indices in the vertex data match the original order of the 'clusters' in the fbx
   // Need to capture the mapping from 'vertex order' to 'joint order' and store it in the 'model'
   // class

   /* Need to figure out how to deal with transforms from nodes further up in the heirarchy than the
   more precisely, how does ozz animation and skeleton deal with this?
   */

   auto skeleton = ozz::animation::Skeleton{};
   archive >> skeleton;

   Log::debug << "---" << std::endl;

   int j = 0;
   for (const auto& name : skeleton.joint_names()) {
      Log::debug << j << "skeleton joint: " << name << std::endl;
      ++j;
   }

   ufbx_free_scene(scene);

   return 0;
}
