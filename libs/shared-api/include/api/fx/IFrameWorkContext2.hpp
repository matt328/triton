#pragma once

namespace tr {

class IFrameworkContext2 {
public:
  IFrameworkContext2() = default;
  virtual ~IFrameworkContext2() = default;

  IFrameworkContext2(const IFrameworkContext2&) = default;
  IFrameworkContext2(IFrameworkContext2&&) = delete;
  auto operator=(const IFrameworkContext2&) -> IFrameworkContext2& = default;
  auto operator=(IFrameworkContext2&&) -> IFrameworkContext2& = delete;

  virtual auto startGameworld() -> void = 0;
  virtual auto startRenderer() -> void = 0;

  virtual auto runMainLoop() -> void = 0;
  virtual auto stop() -> void = 0;
};

}
