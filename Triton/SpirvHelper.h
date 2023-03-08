#pragma once

#include "glslang/SPIRV/GlslangToSpv.h"
#include <vulkan/vulkan.hpp>

class SpirvHelper {
 public:
   SpirvHelper();
   ~SpirvHelper();

   std::vector<uint32_t> compileShader(vk::ShaderStageFlagBits shaderType,
                                       const char* shaderCode) const;

 private:
   static TBuiltInResource initResources();

   static EShLanguage findLanguage(vk::ShaderStageFlagBits shaderType);
};
