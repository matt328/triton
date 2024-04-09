#include "Pipeline.hpp"
#include "GraphicsDevice.hpp"

namespace tr::gfx {
   Pipeline::Pipeline(const GraphicsDevice& graphicsDevice,
                      const vk::PipelineLayoutCreateInfo& pipelineLayoutCreateInfo,
                      const vk::PipelineVertexInputStateCreateInfo,
                      const std::string_view& vertexShaderName,
                      const std::string_view& fragmentShaderName) {

      // Load up shader modules

      // Create the pipeline layout

      // grab the rest from helpers/Pipeline.hpp

      // TODO: move viewport/snezzors into command buffer recording.
      // for now we don't have to recreate the pipeline on window resizes
   }
}