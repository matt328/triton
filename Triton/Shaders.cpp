#include "Shaders.h"
#include "glslang/Public/ShaderLang.h"

ShaderModule::ShaderModule(const std::shared_ptr<vk::raii::Device>& device,
                           const std::string_view filename) {
   compileShaderFile(filename);

   const auto createInfo =
       vk::ShaderModuleCreateInfo{.codeSize = spirv.size(), .pCode = spirv.data()};

   shaderModule = device->createShaderModule(createInfo);
}

void ShaderModule::compileShaderFile(std::string_view filename) {
   if (const auto shaderSource = readShaderFile(filename); !shaderSource.empty()) {
      compileShader(glslangShaderStageFromFileName(filename), shaderSource.c_str());
   } else {
      throw std::runtime_error(std::format("Failed to compile shader file: {}", filename));
   }
}

std::string ShaderModule::readShaderFile(const std::string_view filename) {
   FILE* file;
   fopen_s(&file, filename.data(), "r");

   if (!file) {
      throw std::runtime_error(std::format("Cannot open file: {}", filename));
   }

   fseek(file, 0L, SEEK_END);
   const auto bytesInFile = ftell(file);
   fseek(file, 0L, SEEK_SET);

   const auto buffer = static_cast<char*>(alloca(bytesInFile + 1));
   const size_t bytesRead = fread(buffer, 1, bytesInFile, file);
   fclose(file);

   buffer[bytesRead] = 0;

   // Handle BOM character
   static constexpr unsigned char BOM[] = {0xEF, 0xBB, 0xBF};

   if (bytesRead > 3) {
      if (!memcmp(buffer, BOM, 3))
         memset(buffer, ' ', 3);
   }

   std::string code(buffer);

   // Substitute in any #included files
   while (code.find("#include ") != std::string::npos) {
      const auto pos = code.find("#include ");
      const auto p1 = code.find('<', pos);
      const auto p2 = code.find('>', pos);
      if (p1 == std::string::npos || p2 == std::string::npos || p2 <= p1) {
         printf("Error while loading shader program: %s\n", code.c_str());
         return {};
      }
      const std::string name = code.substr(p1 + 1, p2 - p1 - 1);
      const std::string include = readShaderFile(name.c_str());
      code.replace(pos, p2 - pos + 1, include.c_str());
   }

   return code;
}

void ShaderModule::compileShader(const glslang_stage_t stage, const char* shaderSource) {
   const auto resources = initResources();
   const auto input = glslang_input_t{
       .language = GLSLANG_SOURCE_GLSL,
       .stage = stage,
       .client = GLSLANG_CLIENT_VULKAN,
       .client_version = GLSLANG_TARGET_VULKAN_1_1,
       .target_language = GLSLANG_TARGET_SPV,
       .target_language_version = GLSLANG_TARGET_SPV_1_3,
       .code = shaderSource,
       .default_version = 100,
       .default_profile = GLSLANG_NO_PROFILE,
       .force_default_version_and_profile = false,
       .forward_compatible = false,
       .messages = GLSLANG_MSG_DEFAULT_BIT,
       .resource = reinterpret_cast<const glslang_resource_t*>(&resources),
   };

   const auto shader = glslang_shader_create(&input);

   if (!glslang_shader_preprocess(shader, &input)) {
      // TODO better error reporting
      fprintf(stderr, "GLSL preprocessing failed\n");
      fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
      fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
      printShaderSource(input.code);
      throw std::runtime_error("GLSL preprocessing failed");
   }

   if (!glslang_shader_parse(shader, &input)) {
      // TODO better error reporting
      fprintf(stderr, "GLSL parsing failed\n");
      fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
      fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
      printShaderSource(glslang_shader_get_preprocessed_code(shader));
      throw std::runtime_error("GLSL parsing failed");
   }

   glslang_program_t* program = glslang_program_create();
   glslang_program_add_shader(program, shader);

   if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
      // TODO better error reporting
      fprintf(stderr, "GLSL linking failed\n");
      fprintf(stderr, "\n%s", glslang_program_get_info_log(program));
      fprintf(stderr, "\n%s", glslang_program_get_info_debug_log(program));
      throw std::runtime_error("GLSL linking failed");
   }

   glslang_program_SPIRV_generate(program, stage);

   this->spirv.resize(glslang_program_SPIRV_get_size(program));

   glslang_program_SPIRV_get(program, this->spirv.data());

   if (const auto spirvMessages = glslang_program_SPIRV_get_messages(program)) {
      fprintf(stderr, "%s", spirvMessages);
   }

   glslang_program_delete(program);
   glslang_shader_delete(shader);
}

void ShaderModule::printShaderSource(const char* text) {
   int line = 1;

   printf("\n(%3i) ", line);

   while (text && *text++) {
      if (*text == '\n') {
         printf("\n(%3i) ", ++line);
      } else if (*text == '\r') {
      } else {
         printf("%c", *text);
      }
   }

   printf("\n");
}

glslang_stage_t ShaderModule::glslangShaderStageFromFileName(const std::string_view filename) {
   if (filename.ends_with(".vert"))
      return GLSLANG_STAGE_VERTEX;

   if (filename.ends_with(".frag"))
      return GLSLANG_STAGE_FRAGMENT;

   if (filename.ends_with(".geom"))
      return GLSLANG_STAGE_GEOMETRY;

   if (filename.ends_with(".comp"))
      return GLSLANG_STAGE_COMPUTE;

   if (filename.ends_with(".tesc"))
      return GLSLANG_STAGE_TESSCONTROL;

   if (filename.ends_with(".tese"))
      return GLSLANG_STAGE_TESSEVALUATION;

   return GLSLANG_STAGE_VERTEX;
}

TBuiltInResource ShaderModule::initResources() {
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

   resources.limits.nonInductiveForLoops = 1;
   resources.limits.whileLoops = 1;
   resources.limits.doWhileLoops = 1;
   resources.limits.generalUniformIndexing = 1;
   resources.limits.generalAttributeMatrixVectorIndexing = 1;
   resources.limits.generalVaryingIndexing = 1;
   resources.limits.generalSamplerIndexing = 1;
   resources.limits.generalVariableIndexing = 1;
   resources.limits.generalConstantMatrixVectorIndexing = 1;

   return resources;
}
