#pragma once

#include "bk/Handle.hpp"
#include "dd/render-pass/ComputePass.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class ManagedImage;
class GraphicsPass;

struct PassInfo {
  std::vector<Handle<ManagedImage>> readImages{};
  std::vector<Handle<ManagedImage>> writeImages{};

  std::vector<BufferHandle> readBuffers{};
  std::vector<BufferHandle> writeBuffers{};

  bool hasSideEffects = false;
};

class IFrameGraph {
public:
  IFrameGraph() = default;
  virtual ~IFrameGraph() = default;

  IFrameGraph(const IFrameGraph&) = default;
  IFrameGraph(IFrameGraph&&) = delete;
  auto operator=(const IFrameGraph&) -> IFrameGraph& = default;
  auto operator=(IFrameGraph&&) -> IFrameGraph& = delete;

  virtual auto addPass(Handle<GraphicsPass> passHandle, PassInfo passInfo) -> void = 0;
  virtual auto addPass(Handle<ComputePass> passHandle, PassInfo passInfo) -> void = 0;

  virtual auto bake() -> void = 0;

  virtual auto execute(const Frame* frame) -> void = 0;
};

}
