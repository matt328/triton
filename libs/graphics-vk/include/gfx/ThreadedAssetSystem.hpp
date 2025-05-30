#pragma once

namespace tr {

class IAssetSystem;

class ThreadedAssetSystem {
public:
  explicit ThreadedAssetSystem(std::function<std::unique_ptr<IAssetSystem>()> factory);
  ~ThreadedAssetSystem() = default;

  ThreadedAssetSystem(const ThreadedAssetSystem&) = delete;
  ThreadedAssetSystem(ThreadedAssetSystem&&) = delete;
  auto operator=(const ThreadedAssetSystem&) -> ThreadedAssetSystem& = delete;
  auto operator=(ThreadedAssetSystem&&) -> ThreadedAssetSystem& = delete;

  auto requestStop() -> void;

private:
  std::jthread thread;
  std::unique_ptr<IAssetSystem> system;
};

}
