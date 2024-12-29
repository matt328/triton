#pragma once

namespace tr {
class ShaderBinding;
}

namespace tr {

const std::filesystem::path SHADERS = std::filesystem::current_path() / "assets" / "shaders";

class IPipeline {
public:
  IPipeline() = default;
  virtual ~IPipeline() = default;

  IPipeline(const IPipeline&) = default;
  IPipeline(IPipeline&&) = delete;
  auto operator=(const IPipeline&) -> IPipeline& = default;
  auto operator=(IPipeline&&) -> IPipeline& = delete;

  virtual void bind(const vk::raii::CommandBuffer& cmd) = 0;
  virtual void applyShaderBinding(
      const ShaderBinding& binding,
      uint32_t setIndex,
      const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) = 0;
};

}
