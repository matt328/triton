#pragma once

namespace tr {

class IGuiAdapter;

struct FrameworkConfig {
  glm::ivec2 initialWindowSize;
  std::string windowTitle;
  std::shared_ptr<IGuiAdapter> guiAdapter;
};

}
