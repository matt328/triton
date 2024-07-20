#pragma once

#include "Vertex.hpp"

namespace tr::as {
   class ImageData {
    public:
      std::vector<unsigned char> data{};
      int width{};
      int height{};
      int component{};
      template <class Archive>
      void serialize(Archive& archive) {
         archive(data, width, height, component);
      }
   };

   struct Model {
      std::vector<tr::as::Vertex> vertices{};
      std::vector<uint32_t> indices{};
      std::unordered_map<int, int> jointRemaps{};
      std::vector<glm::mat4> inverseBindPoses{};
      ImageData imageData{};

      [[nodiscard]] auto skinned() const {
         return !inverseBindPoses.empty();
      }

      [[nodiscard]] auto numJoints() const {
         return static_cast<int>(inverseBindPoses.size());
      }

      template <class Archive>
      void serialize(Archive& archive) {
         archive(vertices, indices, jointRemaps, inverseBindPoses, imageData);
      }
   };
}