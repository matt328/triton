#pragma once

#include "dd/DrawContext.hpp"
#include "dd/LogicalImageHandle.hpp"
#include "dd/RenderConfigHandle.hpp"

namespace tr {

enum class RenderPassType : uint8_t {
  ForwardOpaque = 0,
  PostProcess,
  UI,
};

struct RenderPassCreateInfo {
  RenderPassType type;
  std::optional<LogicalImageHandle> colorImageHandle;
  std::optional<glm::vec3> clearColor;
  std::optional<LogicalImageHandle> depthImageHandle;
  std::optional<glm::vec3> depthColor;
  std::optional<std::string> name;
};

class DrawContextFactory;
class Frame;

class RenderPass {
public:
  explicit RenderPass(RenderPassCreateInfo& createInfo);
  ~RenderPass() = default;

  RenderPass(const RenderPass&) = default;
  RenderPass(RenderPass&&) = delete;
  auto operator=(const RenderPass&) -> RenderPass& = default;
  auto operator=(RenderPass&&) -> RenderPass& = delete;

  [[nodiscard]] auto accepts(const RenderConfig& config) const -> bool;

  auto addDrawContext(RenderConfigHandle handle, DrawContext* drawContext) -> void;

  auto execute(const Frame* frame, vk::CommandBuffer& cmdBuffer) -> void;

private:
  std::optional<std::string> debugName;
  std::unordered_map<RenderConfigHandle, DrawContext*> drawContexts;
};

}
