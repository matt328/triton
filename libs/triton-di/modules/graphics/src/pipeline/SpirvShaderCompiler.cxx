#include "SpirvShaderCompiler.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <fstream>
#include <glslang/Public/ShaderLang.h>

namespace tr::gfx::pipe {

   SpirvShaderCompiler::SpirvShaderCompiler(std::shared_ptr<IGraphicsDevice> newGraphicsDevice)
       : graphicsDevice{std::move(newGraphicsDevice)} {
      Log.trace("Constructed SpirvShaderCompiler");
      glslang::InitializeProcess();
   }

   SpirvShaderCompiler::~SpirvShaderCompiler() {
      glslang::FinalizeProcess();
   }

   [[nodiscard]] auto SpirvShaderCompiler::createShaderModule(
       vk::ShaderStageFlagBits shaderType,
       const std::filesystem::path& filename) const -> vk::raii::ShaderModule {
      const auto vertexSpirv = compileShader(shaderType, readShaderFile(filename).data());

      const auto vertexShaderCreateInfo =
          vk::ShaderModuleCreateInfo{.codeSize = 4 * vertexSpirv.size(),
                                     .pCode = vertexSpirv.data()};

      return graphicsDevice->getVulkanDevice()->createShaderModule(vertexShaderCreateInfo);
   }

   auto SpirvShaderCompiler::readShaderFile(const std::filesystem::path& filename) -> std::string {
      if (std::ifstream file(filename.string().data(), std::ios::binary); file.is_open()) {
         file.seekg(0, std::ios::end);

         const std::streampos fileSize = file.tellg();

         file.seekg(0, std::ios::beg);

         std::string shaderCode;
         shaderCode.resize(fileSize);
         file.read(shaderCode.data(), fileSize);

         file.close();
         return shaderCode;
      }
      std::stringstream ss;
      ss << "Failed to read shader from file " << filename.string().data();
      throw std::runtime_error(ss.str());
   }

   auto SpirvShaderCompiler::compileShader(const vk::ShaderStageFlagBits shaderType,
                                           const char* shaderCode) -> std::vector<uint32_t> {
      const EShLanguage stage = findLanguage(shaderType);

      glslang::TShader shader(stage);
      glslang::TProgram program;

      const auto shaderStrings = std::array<const char*, 1>{shaderCode};
      const auto resources = initResources();

      constexpr auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
      shader.setStrings(shaderStrings.data(), 1);

      if (!shader.parse(&resources, 100, false, messages)) {
         std::stringstream ss;
         ss << "Shader Compile fail " << shader.getInfoLog() << " | " << shader.getInfoDebugLog();
         throw std::runtime_error(ss.str());
      }

      program.addShader(&shader);

      if (!program.link(messages)) {
         std::stringstream ss;
         ss << "Shader Compile fail " << shader.getInfoLog() << " | " << shader.getInfoDebugLog();
         throw std::runtime_error(ss.str());
      }

      std::vector<uint32_t> spirv{};
      glslang::SpvOptions spvOptions{.generateDebugInfo = true};
      glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &spvOptions);

      return spirv;
   }

   auto SpirvShaderCompiler::initResources() -> TBuiltInResource {
      TBuiltInResource resources{};
      resources.maxLights = 32;
      resources.maxClipPlanes = 6;
      resources.maxTextureUnits = 32;
      resources.maxTextureCoords = 32;
      resources.maxVertexAttribs = 64;
      resources.maxVertexUniformComponents = 4096;
      resources.maxVaryingFloats = 64;
      resources.maxVertexTextureImageUnits = 32;
      resources.maxCombinedTextureImageUnits = 80;
      resources.maxTextureImageUnits = 32;
      resources.maxFragmentUniformComponents = 4096;
      resources.maxDrawBuffers = 32;
      resources.maxVertexUniformVectors = 128;
      resources.maxVaryingVectors = 8;
      resources.maxFragmentUniformVectors = 16;
      resources.maxVertexOutputVectors = 16;
      resources.maxFragmentInputVectors = 15;
      resources.minProgramTexelOffset = -8;
      resources.maxProgramTexelOffset = 7;
      resources.maxClipDistances = 8;
      resources.maxComputeWorkGroupCountX = 65535;
      resources.maxComputeWorkGroupCountY = 65535;
      resources.maxComputeWorkGroupCountZ = 65535;
      resources.maxComputeWorkGroupSizeX = 1024;
      resources.maxComputeWorkGroupSizeY = 1024;
      resources.maxComputeWorkGroupSizeZ = 64;
      resources.maxComputeUniformComponents = 1024;
      resources.maxComputeTextureImageUnits = 16;
      resources.maxComputeImageUniforms = 8;
      resources.maxComputeAtomicCounters = 8;
      resources.maxComputeAtomicCounterBuffers = 1;
      resources.maxVaryingComponents = 60;
      resources.maxVertexOutputComponents = 64;
      resources.maxGeometryInputComponents = 64;
      resources.maxGeometryOutputComponents = 128;
      resources.maxFragmentInputComponents = 128;
      resources.maxImageUnits = 8;
      resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
      resources.maxCombinedShaderOutputResources = 8;
      resources.maxImageSamples = 0;
      resources.maxVertexImageUniforms = 0;
      resources.maxTessControlImageUniforms = 0;
      resources.maxTessEvaluationImageUniforms = 0;
      resources.maxGeometryImageUniforms = 0;
      resources.maxFragmentImageUniforms = 8;
      resources.maxCombinedImageUniforms = 8;
      resources.maxGeometryTextureImageUnits = 16;
      resources.maxGeometryOutputVertices = 256;
      resources.maxGeometryTotalOutputComponents = 1024;
      resources.maxGeometryUniformComponents = 1024;
      resources.maxGeometryVaryingComponents = 64;
      resources.maxTessControlInputComponents = 128;
      resources.maxTessControlOutputComponents = 128;
      resources.maxTessControlTextureImageUnits = 16;
      resources.maxTessControlUniformComponents = 1024;
      resources.maxTessControlTotalOutputComponents = 4096;
      resources.maxTessEvaluationInputComponents = 128;
      resources.maxTessEvaluationOutputComponents = 128;
      resources.maxTessEvaluationTextureImageUnits = 16;
      resources.maxTessEvaluationUniformComponents = 1024;
      resources.maxTessPatchComponents = 120;
      resources.maxPatchVertices = 32;
      resources.maxTessGenLevel = 64;
      resources.maxViewports = 16;
      resources.maxVertexAtomicCounters = 0;
      resources.maxTessControlAtomicCounters = 0;
      resources.maxTessEvaluationAtomicCounters = 0;
      resources.maxGeometryAtomicCounters = 0;
      resources.maxFragmentAtomicCounters = 8;
      resources.maxCombinedAtomicCounters = 8;
      resources.maxAtomicCounterBindings = 1;
      resources.maxVertexAtomicCounterBuffers = 0;
      resources.maxTessControlAtomicCounterBuffers = 0;
      resources.maxTessEvaluationAtomicCounterBuffers = 0;
      resources.maxGeometryAtomicCounterBuffers = 0;
      resources.maxFragmentAtomicCounterBuffers = 1;
      resources.maxCombinedAtomicCounterBuffers = 1;
      resources.maxAtomicCounterBufferSize = 16384;
      resources.maxTransformFeedbackBuffers = 4;
      resources.maxTransformFeedbackInterleavedComponents = 64;
      resources.maxCullDistances = 8;
      resources.maxCombinedClipAndCullDistances = 8;
      resources.maxSamples = 4;
      resources.maxMeshOutputVerticesNV = 256;
      resources.maxMeshOutputPrimitivesNV = 512;
      resources.maxMeshWorkGroupSizeX_NV = 32;
      resources.maxMeshWorkGroupSizeY_NV = 1;
      resources.maxMeshWorkGroupSizeZ_NV = 1;
      resources.maxTaskWorkGroupSizeX_NV = 32;
      resources.maxTaskWorkGroupSizeY_NV = 1;
      resources.maxTaskWorkGroupSizeZ_NV = 1;
      resources.maxMeshViewCountNV = 4;
      resources.limits.nonInductiveForLoops = true;
      resources.limits.whileLoops = true;
      resources.limits.doWhileLoops = true;
      resources.limits.generalUniformIndexing = true;
      resources.limits.generalAttributeMatrixVectorIndexing = true;
      resources.limits.generalVaryingIndexing = true;
      resources.limits.generalSamplerIndexing = true;
      resources.limits.generalVariableIndexing = true;
      resources.limits.generalConstantMatrixVectorIndexing = true;
      return resources;
   }

   auto SpirvShaderCompiler::findLanguage(const vk::ShaderStageFlagBits shaderType) -> EShLanguage {
      switch (shaderType) {
         case vk::ShaderStageFlagBits::eVertex:
            return EShLangVertex;
         case vk::ShaderStageFlagBits::eTessellationControl:
            return EShLangTessControl;
         case vk::ShaderStageFlagBits::eTessellationEvaluation:
            return EShLangTessEvaluation;
         case vk::ShaderStageFlagBits::eGeometry:
            return EShLangGeometry;
         case vk::ShaderStageFlagBits::eFragment:
            return EShLangFragment;
         case vk::ShaderStageFlagBits::eCompute:
            return EShLangCompute;
         default:
            return EShLangVertex;
      }
   }

}