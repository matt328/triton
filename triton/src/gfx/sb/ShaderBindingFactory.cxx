#include "ShaderBindingFactory.hpp"
#include "gfx/sb/DSShaderBinding.hpp"

#include "gfx/GraphicsDevice.hpp"
#include "gfx/sb/ShaderBinding.hpp"

namespace tr::gfx::sb {
   ShaderBindingFactory::ShaderBindingFactory(const GraphicsDevice& graphicsDevice,
                                              const ds::LayoutFactory& layoutFactory,
                                              const bool useDescriptorBuffers)
       : graphicsDevice{graphicsDevice},
         layoutFactory{layoutFactory},
         useDescriptorBuffers{useDescriptorBuffers} {
   }

   ShaderBindingFactory::~ShaderBindingFactory() {
   }

   auto ShaderBindingFactory::createShaderBinding(ShaderBindingHandle handle)
       -> std::unique_ptr<ShaderBinding> {

      if (handle == ShaderBindingHandle::PerFrame) {
         return std::make_unique<DSShaderBinding>(graphicsDevice.getVulkanDevice(),
                                                  vk::DescriptorType::eUniformBuffer);
      }

      Log::error << "unhandled, uh, handle when creating shaderbinding: " << std::endl;
      assert(true);

      return nullptr;
   }
}
