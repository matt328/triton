#pragma once

namespace tr::gfx::pipe {
   class IShaderCompiler {
    public:
      IShaderCompiler() = default;
      virtual ~IShaderCompiler() = default;

      IShaderCompiler(const IShaderCompiler&) = default;
      IShaderCompiler(IShaderCompiler&&) = delete;
      auto operator=(const IShaderCompiler&) -> IShaderCompiler& = default;
      auto operator=(IShaderCompiler&&) -> IShaderCompiler& = delete;

      [[nodiscard]] virtual auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                                                    const std::filesystem::path& filename) const
          -> vk::raii::ShaderModule = 0;
   };
}
