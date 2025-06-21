#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

struct MockRenderPass : public IRenderPass {
  PassId id;
  PassGraphInfo info;

  MockRenderPass(PassId pid, tr::PassGraphInfo pinfo) : id(pid), info(std::move(pinfo)) {
  }

  auto getId() const -> PassId override {
    return id;
  }

  auto getGraphInfo() const -> PassGraphInfo override {
    return info;
  }

  auto execute(Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void override {};
  auto registerDispatchContext(Handle<IDispatchContext> handle) -> void override {};
};

}
