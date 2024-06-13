#pragma once

namespace tr::util {
   glm::mat4 ozzToGlm(const ozz::math::Float4x4& ozzMatrix) {
      glm::mat4 glmMatrix{};

      alignas(16) float temp[4];

      for (int i = 0; i < 4; ++i) {
         ozz::math::StorePtrU(ozzMatrix.cols[i], temp);
         for (int j = 0; j < 4; ++j) {
            glmMatrix[i][j] = temp[j];
         }
      }
      return glmMatrix;
   }
}