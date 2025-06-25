#pragma once

namespace tr {

struct EditorState {
  struct Assets {
    std::vector<std::string> skeletons;
    std::vector<std::string> animations;
    std::vector<std::string> models;
  };

  bool saved = true;
  Assets assets{};
};

}
