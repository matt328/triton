#pragma once

#include "bk/Handle.hpp"
#include "vk/ResourceManagerHandles.hpp"
#include "gfx/PassGraphInfo.hpp"

namespace tr {

class ManagedImage;
class GraphicsPass;
class ComputePass;
class Frame;

struct PassInfo {
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

  virtual auto addPass(std::unique_ptr<GraphicsPass>&& pass, PassGraphInfo passInfo) -> void = 0;
  virtual auto addPass(std::unique_ptr<ComputePass>&& pass, PassGraphInfo passInfo) -> void = 0;

  [[nodiscard]] virtual auto getGraphicsPass(std::string id) -> std::unique_ptr<GraphicsPass>& = 0;
  [[nodiscard]] virtual auto getComputePass(std::string id) -> std::unique_ptr<ComputePass>& = 0;

  virtual auto bake() -> void = 0;

  virtual auto execute(const Frame* frame) -> FrameGraphResult = 0;
};

}
