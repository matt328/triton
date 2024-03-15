#pragma once

#include "gfx/geometry/Vertex.hpp"
#include "util/KtxImage.hpp"

namespace tr::ctx {

   using Vertex = gfx::Geometry::Vertex;

   struct Primitive {
      std::vector<uint16_t> indices{};
      std::vector<Vertex> vertices{};
   };

   struct Material {
      struct Constants {
         glm::vec4 colorFactors{1};
         glm::vec2 metalRoughFactors{1};
      };

      std::optional<std::unique_ptr<util::KtxImage>> albedoMap;
      std::optional<std::unique_ptr<util::KtxImage>> roughnessMap;
      std::optional<std::unique_ptr<util::KtxImage>> normalMap;
      std::optional<std::unique_ptr<util::KtxImage>> aoMap;

      Constants constants;
   };

   class RenderObjectDataIterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = std::pair<Primitive&, Material&>;
      using difference_type = std::ptrdiff_t;
      using pointer = value_type*;
      using reference = value_type&;

      RenderObjectDataIterator(std::vector<Primitive>& primitives,
                               std::vector<Material>& materials,
                               std::unordered_map<size_t, size_t>& primitivesToMaterials,
                               size_t index)
          : primitives(primitives),
            materials(materials),
            primitivesToMaterials(primitivesToMaterials),
            index(index) {
      }

      RenderObjectDataIterator& operator++() {
         ++index;
         return *this;
      }

      RenderObjectDataIterator operator++(int) {
         auto temp = *this;
         ++(*this);
         return temp;
      }

      bool operator==(const RenderObjectDataIterator& other) const {
         return index == other.index;
      }

      bool operator!=(const RenderObjectDataIterator& other) const {
         return !(*this == other);
      }

      value_type operator*() const {
         size_t primitiveIndex = index;
         size_t materialIndex = primitivesToMaterials.at(primitiveIndex);
         return {primitives[primitiveIndex], materials[materialIndex]};
      }

    private:
      std::vector<Primitive>& primitives;
      std::vector<Material>& materials;
      std::unordered_map<size_t, size_t>& primitivesToMaterials;
      size_t index;
   };

   class RenderObjectData {
    public:
      using Iterator = RenderObjectDataIterator;
      RenderObjectData(){};
      ~RenderObjectData(){};

      RenderObjectData(const RenderObjectData&) = delete;
      RenderObjectData(RenderObjectData&&) = delete;
      RenderObjectData& operator=(const RenderObjectData&) = delete;
      RenderObjectData& operator=(RenderObjectData&&) = delete;

      void addPrimitive(Primitive&& p, Material&& m);

      Iterator begin() {
         return RenderObjectDataIterator{primitives, materials, primitivesToMaterials, 0};
      }

      Iterator end() {
         return RenderObjectDataIterator{primitives,
                                         materials,
                                         primitivesToMaterials,
                                         primitives.size()};
      }

    private:
      std::vector<Primitive> primitives;
      std::vector<Material> materials;
      std::unordered_map<size_t, size_t> primitivesToMaterials;
   };
}
