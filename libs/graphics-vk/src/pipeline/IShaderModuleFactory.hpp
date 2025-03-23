#pragma once

namespace tr {
class IShaderModuleFactory {
public:
  IShaderModuleFactory() = default;
  virtual ~IShaderModuleFactory() = default;

  IShaderModuleFactory(const IShaderModuleFactory&) = default;
  IShaderModuleFactory(IShaderModuleFactory&&) = delete;
  auto operator=(const IShaderModuleFactory&) -> IShaderModuleFactory& = default;
  auto operator=(IShaderModuleFactory&&) -> IShaderModuleFactory& = delete;

  [[nodiscard]] virtual auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                                                const std::filesystem::path& filename) const
      -> vk::raii::ShaderModule = 0;
};
}
