#pragma once

namespace tr {

struct EditorState {
  struct Assets {
    std::unordered_map<std::string, std::string> skeletons;
    std::unordered_map<std::string, std::string> animations;
    std::unordered_map<std::string, std::string> models;
  };

  bool saved = true;
  Assets assets{};
};

}
