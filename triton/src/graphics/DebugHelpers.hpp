#pragma once

namespace Triton {
   template <typename T>
   void setObjectName(T const& handle,
                      [[maybe_unused]] const vk::raii::Device& device,
                      const vk::DebugReportObjectTypeEXT objectType,
                      const std::string_view name) {
      // NOLINTNEXTLINE this is just debug anyway
      const auto debugHandle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(handle));

      [[maybe_unused]] const auto debugNameInfo = vk::DebugMarkerObjectNameInfoEXT{
          .objectType = objectType, .object = debugHandle, .pObjectName = name.data()};
      // TODO: vulkan sdk 261 busted this, something is broken with vulkan_raii, when i look up
      // the function manually it's there but vulkan_raii can't find it
      // device.debugMarkerSetObjectNameEXT(debugNameInfo);
   }
}