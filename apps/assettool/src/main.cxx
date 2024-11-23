#include "as/Model.hpp"

#include "as/ModelConverter.hpp"
#include "as/gltf/GeometryExtractor.hpp"
#include "as/gltf/ModelLoader.hpp"
#include "as/gltf/SkeletonLoader.hpp"
#include "as/gltf/SkinningDataExtractor.hpp"
#include "as/gltf/TextureExtractor.hpp"
#include "as/gltf/TransformParser.hpp"
#include "GlmCereal.hpp"

constexpr int ExpectedArgCount = 5;

auto parseCommandLine(const std::vector<std::string>& args) {
   auto options = std::unordered_map<std::string, std::string>{};

   if (args.size() != ExpectedArgCount) {
      Log.error("Must pass {0} command line args", ExpectedArgCount);
      throw std::runtime_error("Must pass at least 5 command line args");
   }

   options["mode"] = args[1];

   for (size_t i = 2; i < args.size(); ++i) {
      if (const std::string& arg = args[i]; arg.starts_with('-')) {
         if (const auto delimiter_pos = arg.find('='); delimiter_pos != std::string::npos) {
            std::string key = arg.substr(1, delimiter_pos - 1); // Skip the leading '-'
            const std::string value = arg.substr(delimiter_pos + 1);
            options[key] = value;
         }
      }
   }

   return options;
}

auto main(int argc, char* argv[]) -> int {
   std::vector<std::string> args(argv, argv + argc);
   initLogger(spdlog::level::trace, spdlog::level::trace);

   namespace fs = std::filesystem;
   namespace as = tr::as;
   try {
      if (auto options = parseCommandLine(args); options["mode"] == "gltf") {
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

         std::unique_ptr<as::TransformParser> transformParser =
             std::make_unique<as::gltf::TransformParser>();

         std::unique_ptr<as::GeometryExtractor> geometryExtractor =
             std::make_unique<as::gltf::GeometryExtractor>();

         std::unique_ptr<as::TextureExtractor> textureExtractor =
             std::make_unique<as::gltf::TextureExtractor>();

         std::unique_ptr<as::SkinningDataExtractor> skinningDataExtractor =
             std::make_unique<as::gltf::SkinningDataExtractor>();

         std::unique_ptr<as::ModelLoader> modelLoader = std::make_unique<as::gltf::ModelLoader>();

         std::unique_ptr<as::SkeletonLoader> skeletonLoader =
             std::make_unique<as::gltf::SkeletonLoader>();

         auto modelConverter = as::ModelConverter(std::move(transformParser),
                                                  std::move(geometryExtractor),
                                                  std::move(textureExtractor),
                                                  std::move(skinningDataExtractor),
                                                  std::move(modelLoader),
                                                  std::move(skeletonLoader));

         {
            modelConverter.load(as::ModelResources(gltfFile, skeletonFile));
            auto tritonModel = modelConverter.buildTritonModel();

            Log.info("Writing file with serialization version {0}", tr::as::SERIAL_VERSION);

            if (outputFile.extension() == ".json") {
               if (std::ofstream outputStream(outputFile); outputStream) {
                  cereal::JSONOutputArchive jsonOutput(outputStream);
                  jsonOutput(tritonModel);
                  Log.info("Wrote json output file to {0}", outputFile.string());
               } else {
                  Log.error("Failed to open output file {0}", outputFile.string());
               }
            } else if (outputFile.extension() == ".trm") {
               if (std::ofstream outputStream(outputFile); outputStream) {
                  cereal::PortableBinaryOutputArchive binOutput(outputStream);
                  binOutput(tritonModel);
                  Log.info("Wrote binary output file to {0}", outputFile.string());
               } else {
                  Log.error("Failed to open output file {0}", outputFile.string());
               }
            }
         }

      } else {
         Log.error("First arg must be 'gltf' for now");
         return -1;
      }
   } catch (const std::exception& ex) {
      Log.error(ex.what());
      return -1;
   }
   return 0;
}
