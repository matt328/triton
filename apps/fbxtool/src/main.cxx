#include "Converter.hpp"
#include "GltfConverter.hpp"

/*
   TODO:
      1. Create a jointMapping in the same format as the fbx2mesh tool
      - compare generated joint map from my code to the one fbx2mesh generates
      - once that's set up, add it to the cereal thing to write it to the model file
      - actually understand how to use this joint mapping

      - write rendering code for each piece incrementally so we can verify each stage is working
      as it should
      - first render the skeleton, bind pose, and sampled animation keyframes
      - then render the mesh, maybe add a list of joints in the ui so you can select a joint
      and color vertices affected by said joint to confirm the mapping
      - how to render something that debugs when the sampled animations just garble up the mesh?
*/

int main() {

   Log::LogManager::getInstance().setMinLevel(Log::Level::Trace);

   // Load and parse the data
   const auto filename =
       std::filesystem::path{R"(C:\Users\Matt\Projects\game-assets\models\working\Walking.fbx)"};

   const auto gltfFilename = std::filesystem::path{
       R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\walking.gltf)"};
   const auto gltfSkeletonPath = std::filesystem::path{
       R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\skeleton.ozz)"};
   const auto gltfAnimationPath = std::filesystem::path{
       R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\animation.ozz)"};

   auto c = al::Converter{};

   auto g = al::gltf::Converter{};

   auto modelData = c.convertFbx(filename);

   auto gltfModelData = g.convert(gltfFilename, gltfSkeletonPath, gltfAnimationPath);

   // Write out runtime forms of the data
   const auto skeletonPath =
       std::filesystem::path{R"(C:\Users\Matt\Projects\game-assets\models\working\skeleton2.ozz)"};
   {
      auto outputFile = ozz::io::File(skeletonPath.string().c_str(), "wb");
      if (outputFile.opened()) {
         auto outputArchive = ozz::io::OArchive(&outputFile, ozz::GetNativeEndianness());
         outputArchive << modelData.skeleton;
      } else {
         Log::error << "failed to write skeleton file: " << skeletonPath.string() << std::endl;
      }
   }

   const auto animationPath =
       std::filesystem::path{R"(C:\Users\Matt\Projects\game-assets\models\working\animation2.ozz)"};
   {
      auto outputFile = ozz::io::File(animationPath.string().c_str(), "wb");
      if (outputFile.opened()) {
         auto outputArchive = ozz::io::OArchive(&outputFile, ozz::GetNativeEndianness());
         outputArchive << modelData.animation;
      } else {
         Log::error << "failed to write animation file: " << animationPath.string() << std::endl;
      }
   }

   const auto modelPath =
       std::filesystem::path{R"(C:\Users\Matt\Projects\game-assets\models\working\model.bin)"};
   {
      auto os = std::ofstream(modelPath, std::ios::binary);
      cereal::BinaryOutputArchive output(os);
      output(modelData.model);
   }

   return 0;
}
