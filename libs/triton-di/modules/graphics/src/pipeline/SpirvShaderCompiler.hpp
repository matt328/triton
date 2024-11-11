#pragma once

#include <glslang/Public/ShaderLang.h>

#include "IShaderCompiler.hpp"
#include "gfx/IGraphicsDevice.hpp"

namespace tr::gfx::pipe {
   class SpirvShaderCompiler : public IShaderCompiler {
    public:
      explicit SpirvShaderCompiler(std::shared_ptr<IGraphicsDevice> newGraphicsDevice);

      [[nodiscard]] auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                                            const std::filesystem::path& filename) const
          -> vk::raii::ShaderModule override;

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;

      static auto compileShader(vk::ShaderStageFlagBits shaderType, const char* shaderCode)
          -> std::vector<uint32_t>;
      static auto readShaderFile(const std::filesystem::path& filename) -> std::string;
      static auto initResources() -> TBuiltInResource;
      static auto findLanguage(vk::ShaderStageFlagBits shaderType) -> EShLanguage;
   };
}