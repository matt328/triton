#include "as/Model.hpp"

#include "as/ModelConverter.hpp"
#include "as/gltf/GeometryExtractor.hpp"
#include "as/gltf/ModelLoader.hpp"
#include "as/gltf/SkeletonLoader.hpp"
#include "as/gltf/SkinningDataExtractor.hpp"
#include "as/gltf/TextureExtractor.hpp"
#include "as/gltf/TransformParser.hpp"
#include <cereal/archives/binary.hpp>

auto parseCommandLine(const int argc, char* argv[]) {
   auto options = std::unordered_map<std::string, std::string>{};

   if (argc != 5) {
      Log.error("Must pass 3 command line args");
      throw std::runtime_error("Must pass at least 3 command line args");
   }

   options["mode"] = argv[1];

   for (int i = 2; i < argc; ++i) {
      if (std::string arg = argv[i]; arg.starts_with('-')) {
         if (const auto delimiter_pos = arg.find('='); delimiter_pos != std::string::npos) {
            std::string key = arg.substr(1, delimiter_pos - 1); // Skip the leading '-'
            const std::string value = arg.substr(delimiter_pos + 1);
            options[key] = value;
         }
      }
   }

   return options;
}

int main(int argc, char* argv[]) {

   initLogger();
   Log.set_level(spdlog::level::debug);

   namespace fs = std::filesystem;
   using namespace tr::as;

   if (auto options = parseCommandLine(argc, argv); options["mode"] == "gltf") {
      const auto gltfFileStr = options["f"];
      auto gltfFile = fs::path{gltfFileStr};

      if (gltfFile.is_relative()) {
         gltfFile = absolute(gltfFile);
      }

      auto skeletonFile = fs::path{options["s"]};
      if (skeletonFile.is_relative()) {
         skeletonFile = absolute(skeletonFile);
      }

      auto outputFile = fs::path{options["o"]};
      if (outputFile.is_relative()) {
         outputFile = absolute(outputFile);
      }

      Log.info("Converting Gltf file, input: {0}, skeleton file: {1}",
               gltfFile.string(),
               skeletonFile.string());

      std::unique_ptr<TransformParser> transformParser = std::make_unique<gltf::TransformParser>();

      std::unique_ptr<GeometryExtractor> geometryExtractor =
          std::make_unique<gltf::GeometryExtractor>();

      std::unique_ptr<TextureExtractor> textureExtractor =
          std::make_unique<gltf::TextureExtractor>();

      std::unique_ptr<SkinningDataExtractor> skinningDataExtractor =
          std::make_unique<gltf::SkinningDataExtractor>();

      std::unique_ptr<ModelLoader> modelLoader = std::make_unique<gltf::ModelLoader>();

      std::unique_ptr<SkeletonLoader> skeletonLoader = std::make_unique<gltf::SkeletonLoader>();

      auto modelConverter = ModelConverter(std::move(transformParser),
                                           std::move(geometryExtractor),
                                           std::move(textureExtractor),
                                           std::move(skinningDataExtractor),
                                           std::move(modelLoader),
                                           std::move(skeletonLoader));

      try {
         {
            modelConverter.load(gltfFile, skeletonFile);
            auto tritonModel = modelConverter.buildTritonModel();

            Log.info("Writing file with serialization version {0}", tr::as::SERIAL_VERSION);

            if (outputFile.extension() == ".json") {
               auto os = std::ofstream(outputFile);
               cereal::JSONOutputArchive jsonOutput(os);
               jsonOutput(tritonModel);
            } else if (outputFile.extension() == ".trm") {
               auto os = std::ofstream(outputFile);
               cereal::BinaryOutputArchive binOutput(os);
               binOutput(tritonModel);
            }
         }
         Log.info("Wrote json output file to {0}", outputFile.string());
      } catch (const std::exception& ex) { Log.error(ex.what()); }
   } else {
      Log.error("First arg must be 'gltf' for now");
      return -1;
   }

   return 0;
}
