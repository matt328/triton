#pragma once

namespace graphics {
   template <typename T>
   void setObjectName(T const& handle,
                      const vk::raii::Device& device,
                      const vk::DebugReportObjectTypeEXT objectType,
                      const std::string_view name) {
      // NOLINTNEXTLINE this is just debug anyway
      const auto debugHandle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(handle));

      const auto debugNameInfo = vk::DebugMarkerObjectNameInfoEXT{
          .objectType = objectType, .object = debugHandle, .pObjectName = name.data()};
      device.debugMarkerSetObjectNameEXT(debugNameInfo);
   }
}