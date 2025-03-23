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

  [[nodiscard]] virtual auto getPipeline() const -> vk::Pipeline = 0;
  [[nodiscard]] virtual auto getPipelineLayout() const -> vk::PipelineLayout = 0;
};

}
