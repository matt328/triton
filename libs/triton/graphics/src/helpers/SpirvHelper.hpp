#pragma once

#include <glslang/Public/ShaderLang.h>

namespace tr::gfx::Helpers {

   class SpirvHelper {
    public:
      explicit SpirvHelper(const vk::raii::Device& device);
      ~SpirvHelper();

      SpirvHelper(const SpirvHelper&) = default;
      SpirvHelper(SpirvHelper&&) = delete;
      auto operator=(const SpirvHelper&) -> SpirvHelper& = delete;
      auto operator=(SpirvHelper&&) -> SpirvHelper& = delete;

      static auto compileShader(vk::ShaderStageFlagBits shaderType, const char* shaderCode)
          -> std::vector<uint32_t>;

      static auto readShaderFile(const std::filesystem::path& filename) -> std::string;

      [[nodiscard]] auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                                            const std::filesystem::path& filename) const
          -> vk::raii::ShaderModule;

    private:
      const vk::raii::Device& device;
      static auto initResources() -> TBuiltInResource;

      static auto findLanguage(vk::ShaderStageFlagBits shaderType) -> EShLanguage;
   };
}