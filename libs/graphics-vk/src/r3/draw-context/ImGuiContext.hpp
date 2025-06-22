#pragma once

#include "IDispatchContext.hpp"
#include "r3/draw-context/DispatchCreateInfos.hpp"

namespace tr {

class IGuiCallbackRegistrar;

class ImGuiContext : public IDispatchContext {
public:
  ImGuiContext(ContextId newId,
               std::shared_ptr<BufferSystem> newBufferSystem,
               std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
               ImGuiContextCreateInfo newCreateInfo);
  ~ImGuiContext() = default;

  ImGuiContext(const ImGuiContext&) = default;
  ImGuiContext(ImGuiContext&&) = delete;
  auto operator=(const ImGuiContext&) -> ImGuiContext& = default;
  auto operator=(ImGuiContext&&) -> ImGuiContext& = delete;

  auto bind(const Frame* frame,
            vk::raii::CommandBuffer& commandBuffer,
            const vk::raii::PipelineLayout& layout) -> void override;

  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void override;

  auto getPushConstantSize() -> size_t override;

  [[nodiscard]] auto getGraphInfo() const -> PassGraphInfo override;

private:
  std::shared_ptr<IGuiCallbackRegistrar> guiCallbackRegistrar;
  ImGuiContextCreateInfo createInfo;
};

}
