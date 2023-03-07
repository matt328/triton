#include "DefaultPipeline.h"

#include "Log.h"
#include "SpirvHelper.h"
#include "Utils.h"

DefaultPipeline::DefaultPipeline() {

   auto helper = std::make_unique<SpirvHelper>();

   const auto vertFilename = (Paths::SHADERS / "shader.vert").string();
   const auto fragFilename = (Paths::SHADERS / "shader.frag").string();

   auto vertShaderCode = readShaderFile(vertFilename);
   auto fragShaderCode = readShaderFile(fragFilename);

   const auto vertexSpirv =
       helper->compileShader(vk::ShaderStageFlagBits::eVertex, vertShaderCode.data());
   Log::core->debug("Compiled shader {}", vertFilename);

   const auto fragmentSpirv =
       helper->compileShader(vk::ShaderStageFlagBits::eFragment, fragShaderCode.data());
   Log::core->debug("Compiled shader {}", fragFilename);
}

DefaultPipeline::~DefaultPipeline() {
}
