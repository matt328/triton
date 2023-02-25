#pragma once

#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/glslang_c_interface.h>
#include <memory>
#include <string_view>
#include <vulkan/vulkan_raii.hpp>

class ShaderModule {
 public:
   /**
    * \brief Constructs a shader module ready to be used.
    * \param device shared_ptr reference of vk::raii::Device
    * \param filename Absolute path to the shader file
    */
   explicit ShaderModule(const std::shared_ptr<vk::raii::Device>& device,
                         const std::string_view filename);

 private:
   std::vector<unsigned int> spirv;
   vk::raii::ShaderModule shaderModule = nullptr;

   void compileShaderFile(std::string_view filename);

   std::string readShaderFile(const std::string_view filename);

   void compileShader(glslang_stage_t stage, const char* shaderSource);

   static void printShaderSource(const char* text);

   static glslang_stage_t glslangShaderStageFromFileName(std::string_view filename);

   static TBuiltInResource initResources();
};
