#include "GltfConverter.hpp"
#include <memory>

namespace al::gltf {
   Converter::~Converter() noexcept {
   }

   auto Converter::convert(const std::filesystem::path& gltf,
                           const std::filesystem::path& skeletonPath,
                           const std::filesystem::path& animation) -> ModelData {

      auto skeleton = ozz::animation::Skeleton{};
      {
         auto file = ozz::io::File{skeletonPath.string().c_str(), "rb"};

         if (!file.opened()) {
            throw std::runtime_error("Error reading skeleton file");
         }

         ozz::io::IArchive archive(&file);
         archive >> skeleton;
      }

      for (const auto& name : skeleton.joint_names()) {
         Log::debug << "gltf skeleton joint: " << name << std::endl;
      }

      /*
         This skeleton contains extra nodes that aren't joints.
         Load in the animation, and sample it at t=0, and run the sampling job and ltm job to get
         the resulting matrices. Check how many matrices are generated, and try to figure out how
         they match up to the joints in the skin's joint list.
         Should be able to get map of joint name to it's matrix, as well as a map of joint name to
         it's index in the skin's joint list.
         If gltf is like fbx there might be joint nodes that have to be there for the calculation,
         but aren't included in the skin's joint list or considered in the vertex attribute's list

         [ ] Need to iterate over all the joints in the skeleton, in heirarchical order,
         and place each one's position in the master list in a map, excluding the joints that don't
         affect any vertices.

         [ ] Calculate and add the joints' inverse bind poses to a parallel list
         whose order mirrors that of the values of the jointRemaps map.

         Once those two pieces are added correctly, I think we should be ready to render.

         sample_skinning.cc L105 shows how the jointRemaps and inverse bind poses should be used.
         The output of the ltm job does need multiplied by the inverse bind poses, and re ordered
         according to the joint_remaps, it can't just be passed directly to the shader.

         According to the gltf example, we include joint matrices for each joint listed in the skin,
         and in that order.
         https://github.com/SaschaWillems/Vulkan/blob/master/examples/gltfskinning/gltfskinning.cpp#L502

         Also need to understand what all does the animation lib do? Does it know about the nodes'
         TRS or matrices? What about inverse bind matrices? Should I go back to rolling my own in
         order to get a better understanding of how it works and then incorporate the library?

         Need to do this incrementally.
         - Render the skeleton in bind pose.
         - Use the time slider to render the skeleton in sampled animation poses
         - Render the Mesh in bind pose with the skeleton underneath
         - List the joints in the ui, and select a joint to color it, and the vertices affected by
         it
         - Next transform the vertices along with the skeleton and render the mesh in sampled
         animation poses.
      */

      return ModelData{};
   }
}