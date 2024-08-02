#pragma once

#include <glslang/Public/ShaderLang.h>

namespace tr::gfx::Helpers {

   class SpirvHelper {
    public:
      explicit SpirvHelper(const vk::raii::Device& device);
      ~SpirvHelper();

      SpirvHelper(const SpirvHelper&) = default;
      SpirvHelper(SpirvHelper&&) = delete;
      SpirvHelper& operator=(const SpirvHelper&) = delete;
      SpirvHelper& operator=(SpirvHelper&&) = delete;

      static std::vector<uint32_t> compileShader(vk::ShaderStageFlagBits shaderType,
                                                 const char* shaderCode);

      static auto readShaderFile(const std::filesystem::path& filename) -> std::string;

      [[nodiscard]] auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                                            const std::filesystem::path& filename) const
          -> vk::raii::ShaderModule;

    private:
      const vk::raii::Device& device;
      static TBuiltInResource initResources();

      static EShLanguage findLanguage(vk::ShaderStageFlagBits shaderType);
   };
}