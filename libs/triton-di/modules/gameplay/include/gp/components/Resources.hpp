#pragma once

namespace tr {
struct WindowDimensions {
  uint32_t width{}, height{};
};

struct CurrentCamera {
  entt::entity currentCamera;
};
}
