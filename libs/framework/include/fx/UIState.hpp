#pragma once

namespace tr {

struct UIState {

  struct Assets {
    std::vector<std::string> skeletons;
    std::vector<std::string> animations;
    std::vector<std::string> models;
  };

  bool saved = true;
  Assets assets{};
};

}
