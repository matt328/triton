#pragma once
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "ImmediateContext.h"
#include "ModelData.h"
#include "Texture.h"
#include "TextureFactory.h"
#include "Vertex.h"

#include <string_view>
#include <tiny_gltf.h>

#include <vulkan/vulkan_raii.hpp>

namespace Models {

   class Model {
    public:
      explicit Model(const std::string_view& filename);

      

    private:
      std::string path;
      std::vector<std::unique_ptr<Texture>> textures;

      void loadNode(Node* parent,
                    const tinygltf::Node& node,
                    uint32_t nodeIndex,
                    const tinygltf::Model& model,
                    std::vector<uint32_t>& indexBuffer,
                    std::vector<Vertex>& vertexBuffer,
                    float globalscale);
   };

}
