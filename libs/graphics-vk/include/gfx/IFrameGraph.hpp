#pragma once

#include "bk/Handle.hpp"
#include "r3/ComponentIds.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class ManagedImage;
class GraphicsPass;
class ComputePass;
class Frame;
class IRenderPass;
struct PassGraphInfo;

struct PassInfo2 {
  std::vector<Handle<ManagedImage>> readImages{};
  std::vector<Handle<ManagedImage>> writeImages{};

  std::vector<BufferHandle> readBuffers{};
  std::vector<BufferHandle> writeBuffers{};

  bool hasSideEffects = false;
};

struct FrameGraphResult {
  std::vector<vk::CommandBuffer> commandBuffers;
};

class IFrameGraph {
public:
  IFrameGraph() = default;
  virtual ~IFrameGraph() = default;

  IFrameGraph(const IFrameGraph&) = default;
  IFrameGraph(IFrameGraph&&) = delete;
  auto operator=(const IFrameGraph&) -> IFrameGraph& = default;
  auto operator=(IFrameGraph&&) -> IFrameGraph& = delete;

  virtual auto addPass(std::unique_ptr<IRenderPass>&& pass) -> void = 0;
  [[nodiscard]] virtual auto getPass(PassId id) -> std::unique_ptr<IRenderPass>& = 0;

  virtual auto bake() -> void = 0;

  virtual auto execute(const Frame* frame) -> FrameGraphResult = 0;
};

}
