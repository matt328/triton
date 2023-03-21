#pragma once
#include "vma_raii.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace vk {
   namespace raii {
      class DescriptorSet;
      class Device;
   }
}

struct Primitive {
   uint32_t firstIndex;
   uint32_t indexCount;
   uint32_t firstVertex;
   uint32_t vertexCount;

   struct Dimensions {
      glm::vec3 min = glm::vec3(FLT_MAX);
      glm::vec3 max = glm::vec3(-FLT_MAX);
      glm::vec3 size;
      glm::vec3 center;
      float radius;
   } dimensions;

   void setDimensions(glm::vec3 min, glm::vec3 max);

   Primitive(const uint32_t firstIndex, const uint32_t indexCount)
       : firstIndex(firstIndex)
       , indexCount(indexCount)
       , firstVertex(0)
       , vertexCount(0)
       , dimensions() {
   }
};

struct Mesh {
   std::vector<Primitive*> primitives;
   std::string name;

   struct UniformBuffer {
      std::unique_ptr<vma::raii::AllocatedBuffer> buffer;
      std::unique_ptr<vk::raii::DescriptorSet> descriptorSet;
      vk::DescriptorBufferInfo descriptor;
      void* mapped;
   } uniformBuffer;

   struct UniformBlock {
      glm::mat4 matrix;
   } uniformBlock;

   Mesh(const vk::raii::Device& device, glm::mat4 matrix);
   ~Mesh();
};

namespace Models {
   struct Node {
      Node* parent;
      uint32_t index;
      std::vector<Node*> children;
      glm::mat4 matrix;
      std::string name;
      Mesh* mesh;
      glm::vec3 translation{};
      glm::vec3 scale{1.0f};
      glm::quat rotation{};
      glm::mat4 localMatrix();
      glm::mat4 getMatrix();
      void update();
      ~Node();
   };
}
