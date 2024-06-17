#pragma once

#include <glslang/Public/ShaderLang.h>

namespace tr::gfx::Helpers {

   class SpirvHelper {
    public:
      SpirvHelper(const vk::raii::Device& device);
      ~SpirvHelper();

      SpirvHelper(const SpirvHelper&) = default;
      SpirvHelper(SpirvHelper&&) = delete;
      SpirvHelper& operator=(const SpirvHelper&) = delete;
      SpirvHelper& operator=(SpirvHelper&&) = delete;

      std::vector<uint32_t> compileShader(vk::ShaderStageFlagBits shaderType,
                                          const char* shaderCode) const;

      auto readShaderFile(const std::filesystem::path& filename) -> std::string;

      auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                              const std::filesystem::path& filename) -> vk::raii::ShaderModule;

    private:
      const vk::raii::Device& device;
      static TBuiltInResource initResources();

      static EShLanguage findLanguage(vk::ShaderStageFlagBits shaderType);
   };
}