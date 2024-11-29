#pragma once

namespace tr::gfx {
   struct SwapchainSupportDetails {
      vk::SurfaceCapabilitiesKHR capabilities;
      std::vector<vk::SurfaceFormatKHR> formats;
      std::vector<vk::PresentModeKHR> presentModes;
   };
 }
