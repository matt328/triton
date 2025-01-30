#pragma once

namespace tr {
class IContext;

struct FrameworkConfig {
  glm::ivec2 initialWindowSize;
  std::string windowTitle;
};

class ComponentFactory {
public:
  static auto getContext(const FrameworkConfig& config) -> std::shared_ptr<IContext>;
};

}
