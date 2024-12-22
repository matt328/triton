#pragma once

#include "cm/Handles.hpp"

namespace tr {

   struct GeometryHandle {
      size_t handle;
      Topology topology;

      auto operator==(const GeometryHandle& other) const -> bool {
         return handle == other.handle && topology == other.topology;
      }
   };

   using ImageHandle = size_t;
   using TexturedGeometryHandle = std::unordered_map<GeometryHandle, ImageHandle>;

   struct TritonModelData {
    private:
      GeometryHandle geometryHandle;
      ImageHandle imageHandle;
      std::optional<SkinData> skinData = std::nullopt;

    public:
      TritonModelData(const GeometryHandle geometryHandle,
                      const ImageHandle imageHandle,
                      std::optional<SkinData> skinData)
          : geometryHandle(geometryHandle),
            imageHandle(imageHandle),
            skinData(std::move(skinData)) {
      }

      [[nodiscard]] auto getGeometryHandle() const -> const GeometryHandle& {
         return geometryHandle;
      }

      [[nodiscard]] auto getImageHandle() const -> const ImageHandle& {
         return imageHandle;
      }

      [[nodiscard]] auto getSkinData() const -> const std::optional<SkinData>& {
         return skinData;
      }
   };

}

namespace std {
   template <>
   struct hash<tr::GeometryHandle> {
      auto operator()(const tr::GeometryHandle& geometryHandle) const -> size_t {
         // Combine hashes of handle and topology
         size_t hash1 = std::hash<size_t>{}(geometryHandle.handle);
         size_t hash2 = std::hash<int>{}(
             static_cast<int>(geometryHandle.topology)); // Assuming Topology is an enum
         return hash1 ^ (hash2 << 1);                    // Combine hash values
      }
   };
}