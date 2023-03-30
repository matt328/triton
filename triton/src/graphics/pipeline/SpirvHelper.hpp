#pragma once

class SpirvHelper {
 public:
   SpirvHelper();
   ~SpirvHelper();

   SpirvHelper(const SpirvHelper&) = default;
   SpirvHelper(SpirvHelper&&) = delete;
   SpirvHelper& operator=(const SpirvHelper&) = default;
   SpirvHelper& operator=(SpirvHelper&&) = delete;

   std::vector<uint32_t> compileShader(vk::ShaderStageFlagBits shaderType,
                                       const char* shaderCode) const;

 private:
   static TBuiltInResource initResources();

   static EShLanguage findLanguage(vk::ShaderStageFlagBits shaderType);
};
