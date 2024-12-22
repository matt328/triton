#pragma once

namespace tr {
   struct QueueFamilyIndices {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> graphicsFamilyCount;
      std::vector<float> graphicsFamilyPriorities;
      std::optional<uint32_t> presentFamily;
      std::optional<uint32_t> presentFamilyCount;
      std::vector<float> presentFamilyPriorities;
      std::optional<uint32_t> transferFamily;
      std::optional<uint32_t> transferFamilyCount;
      std::vector<float> transferFamilyPriorities;
      std::optional<uint32_t> computeFamily;
      std::optional<uint32_t> computeFamilyCount;
      std::vector<float> computeFamilyPriorities;

      [[nodiscard]] bool isComplete() const {
         return graphicsFamily.has_value() && presentFamily.has_value() &&
                transferFamily.has_value() && computeFamily.has_value();
      }
   };
   }