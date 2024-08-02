#pragma once

#include "cm/Handles.hpp"

namespace tr::gfx::geo {

   using GeometryHandle = size_t;
   using ImageHandle = size_t;
   using TexturedGeometryHandle = std::unordered_map<GeometryHandle, ImageHandle>;

   struct TritonModelData {
    private:
      GeometryHandle geometryHandle;
      ImageHandle imageHandle;
      std::optional<cm::SkinData> skinData = std::nullopt;

    public:
      TritonModelData(const GeometryHandle geometryHandle,
                      const ImageHandle imageHandle,
                      std::optional<cm::SkinData> skinData)
          : geometryHandle(geometryHandle),
            imageHandle(imageHandle),
            skinData(std::move(skinData)) {
      }

      [[nodiscard]] const GeometryHandle& getGeometryHandle() const {
         return geometryHandle;
      }

      [[nodiscard]] const ImageHandle& getImageHandle() const {
         return imageHandle;
      }

      [[nodiscard]] const std::optional<cm::SkinData>& getSkinData() const {
         return skinData;
      }
   };

}
