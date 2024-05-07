#include "DSShaderBinding.hpp"

namespace tr::gfx::sb {

   DSShaderBinding::DSShaderBinding(const vk::raii::Device& device,
                                    vk::DescriptorType descriptorType)
       : ShaderBinding{}, device{device}, descriptorType{descriptorType} {
   }

   void DSShaderBinding::bindBuffer(const mem::Buffer& buffer, int binding) {
   }
}