#pragma once

namespace ed {

struct DialogRenderContext {
  struct DropdownData {
    std::vector<std::string> items;
    int* selectedIndex = nullptr;
  };

  std::unordered_map<std::string, DropdownData> dropdownMap;
};

}
