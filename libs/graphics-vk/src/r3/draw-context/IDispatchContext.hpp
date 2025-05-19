#pragma once

namespace tr {

class Frame;
class BufferSystem;

class IDispatchContext {
public:
  explicit IDispatchContext(std::string newId, std::shared_ptr<BufferSystem> newBufferSystem)
      : id{std::move(newId)}, bufferSystem{std::move(newBufferSystem)} {
  }
  virtual ~IDispatchContext() = default;

  IDispatchContext(const IDispatchContext&) = default;
  IDispatchContext(IDispatchContext&&) = delete;
  auto operator=(const IDispatchContext&) -> IDispatchContext& = default;
  auto operator=(IDispatchContext&&) -> IDispatchContext& = delete;

  virtual auto bind(const Frame* frame,
                    vk::raii::CommandBuffer& commandBuffer,
                    const vk::raii::PipelineLayout& layout) -> void = 0;
  virtual auto dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void = 0;

  virtual auto getPushConstantSize() -> size_t = 0;

protected:
  std::string id;
  std::shared_ptr<BufferSystem> bufferSystem;
};

}
