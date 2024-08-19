#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

   class SkinningDataExtractor final : public as::SkinningDataExtractor {
    public:
      SkinningDataExtractor() = default;
      ~SkinningDataExtractor() noexcept override;

      SkinningDataExtractor(const SkinningDataExtractor&) = delete;
      SkinningDataExtractor& operator=(const SkinningDataExtractor&) = delete;

      SkinningDataExtractor(SkinningDataExtractor&&) = delete;
      SkinningDataExtractor& operator=(SkinningDataExtractor&&) = delete;

      void execute(const tinygltf::Model& model,
                   const ozz::animation::Skeleton& skeleton,
                   Model& tritonModel) override;
   };

}
