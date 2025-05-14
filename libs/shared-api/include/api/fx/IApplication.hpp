#pragma once

namespace tr {

class IApplication {
public:
  IApplication() = default;
  virtual ~IApplication() = default;

  IApplication(const IApplication&) = default;
  IApplication(IApplication&&) = delete;
  auto operator=(const IApplication&) -> IApplication& = default;
  auto operator=(IApplication&&) -> IApplication& = delete;

  virtual auto onStart() -> void = 0;
  virtual auto onUpdate() -> void = 0;
  virtual auto onShutdown() -> void = 0;
};

}
