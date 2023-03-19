#pragma once
#include <functional>
#include <glm/fwd.hpp>

typedef std::function<glm::mat4()> MvpProvider;

class DataProvider {
 public:
  DataProvider() = delete;
  ~DataProvider() = default;

  void registerMvpProvider(const MvpProvider& func) const { this->mvpProvider = func; }

  glm::mat4 getMvp() {
    if (mvpProvider) {
      return mvpProvider();
    }
  }

 private:
  MvpProvider& mvpProvider;
};