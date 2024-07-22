#pragma once

#include "cm/Handles.hpp"

namespace tr::gfx::geo {

   using GeometryHandle = size_t;
   using ImageHandle = size_t;
   using TexturedGeometryHandle = std::unordered_map<GeometryHandle, ImageHandle>;

   struct AnimationData {
    private:
      std::unordered_map<int, int> jointMap;
      std::vector<glm::mat4> inverseBindMatrices;

    public:
      AnimationData(std::unordered_map<int, int> jointMap,
                    std::vector<glm::mat4> inverseBindMatrices)
          : jointMap(std::move(jointMap)), inverseBindMatrices(std::move(inverseBindMatrices)) {
      }

      [[nodiscard]] const std::unordered_map<int, int>& getJointMap() const {
         return jointMap;
      }

      [[nodiscard]] const std::vector<glm::mat4>& getInverseBindMatrices() const {
         return inverseBindMatrices;
      }
   };

   struct TritonModelData {
    private:
      GeometryHandle geometryHandle;
      ImageHandle imageHandle;
      std::optional<AnimationData> animationData = std::nullopt;

    public:
      TritonModelData(GeometryHandle geometryHandle,
                      ImageHandle imageHandle,
                      std::optional<AnimationData> animationData)
          : geometryHandle(std::move(geometryHandle)),
            imageHandle(std::move(imageHandle)),
            animationData(std::move(animationData)) {
      }

      [[nodiscard]] const GeometryHandle& getGeometryHandle() const {
         return geometryHandle;
      }

      [[nodiscard]] const ImageHandle& getImageHandle() const {
         return imageHandle;
      }

      [[nodiscard]] const std::optional<AnimationData>& getAnimationData() const {
         return animationData;
      }
   };

   struct SkinnedGeometryData {
      GeometryHandle geometryHandle;
      ImageHandle imageHandle;
      cm::SkeletonHandle skeletonHandle;
      cm::AnimationHandle animationHandle;
      std::unordered_map<int, int> jointMap;
      std::vector<glm::mat4> inverseBindMatrices;
   };

}
