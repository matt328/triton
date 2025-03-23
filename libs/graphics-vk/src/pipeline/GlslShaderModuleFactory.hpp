#pragma once

#include <glslang/Public/ShaderLang.h>

#include "IShaderModuleFactory.hpp"
#include "vk/core/Device.hpp"

namespace tr {
class GlslShaderModuleFactory : public IShaderModuleFactory {
public:
  explicit GlslShaderModuleFactory(std::shared_ptr<Device> newDevice);
  ~GlslShaderModuleFactory();

  GlslShaderModuleFactory(const GlslShaderModuleFactory&) = default;
  GlslShaderModuleFactory(GlslShaderModuleFactory&&) = delete;
  auto operator=(const GlslShaderModuleFactory&) -> GlslShaderModuleFactory& = default;
  auto operator=(GlslShaderModuleFactory&&) -> GlslShaderModuleFactory& = delete;

  [[nodiscard]] auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                                        const std::filesystem::path& filename) const
      -> vk::raii::ShaderModule override;

private:
  std::shared_ptr<Device> device;

  static auto compileShader(vk::ShaderStageFlagBits shaderType, const char* shaderCode)
      -> std::vector<uint32_t>;
  static auto readShaderFile(const std::filesystem::path& filename) -> std::string;
  static auto initResources() -> TBuiltInResource;
  static auto findLanguage(vk::ShaderStageFlagBits shaderType) -> EShLanguage;
};
}
