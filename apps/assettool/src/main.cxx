#include "as/Model.hpp"

#include "as/ModelConverter.hpp"
#include "as/gltf/GltfGeometryExtractor.hpp"
#include "as/gltf/GltfModelLoader.hpp"
#include "as/gltf/GltfSkeletonLoader.hpp"
#include "as/gltf/GltfSkinningDataExtractor.hpp"
#include "as/gltf/GltfTextureExtractor.hpp"
#include "as/gltf/GltfTransformParser.hpp"
#include "GlmCereal.hpp"

auto parseCommandLine(const std::vector<std::string>& args) {
  auto options = std::unordered_map<std::string, std::string>{};

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
  try {
    if (auto options = parseCommandLine(args); options["mode"] == "gltf") {
      const auto gltfFileStr = options["f"];
      auto gltfFile = fs::path{gltfFileStr};

      if (gltfFile.is_relative()) {
        gltfFile = absolute(gltfFile);
      }

      std::optional<fs::path> skeletonFile;
      if (options.contains("s")) {
        skeletonFile = std::make_optional(fs::path{options["s"]});
        if (skeletonFile.value().is_relative()) {
          skeletonFile = std::make_optional(absolute(skeletonFile.value()));
        }
      }

      auto outputFile = fs::path{options["o"]};
      if (outputFile.is_relative()) {
        outputFile = absolute(outputFile);
      }

      Log.info("Converting Gltf file, input: {0}, skeleton file: {1}",
               gltfFile.string(),
               skeletonFile.has_value() ? skeletonFile.value().string() : "none provided");

      std::unique_ptr<as::ITransformParser> transformParser =
          std::make_unique<as::GltfTransformParser>();

      std::unique_ptr<as::IGeometryExtractor> geometryExtractor =
          std::make_unique<as::GltfGeometryExtractor>();

      std::unique_ptr<as::ITextureExtractor> textureExtractor =
          std::make_unique<as::GltfTextureExtractor>();

      std::unique_ptr<as::ISkinningDataExtractor> skinningDataExtractor =
          std::make_unique<as::GltfSkinningDataExtractor>();

      std::unique_ptr<as::IModelLoader> modelLoader = std::make_unique<as::GltfModelLoader>();

      std::unique_ptr<as::ISkeletonLoader> skeletonLoader =
          std::make_unique<as::GltfSkeletonLoader>();

      auto modelConverter = as::ModelConverter(std::move(transformParser),
                                               std::move(geometryExtractor),
                                               std::move(textureExtractor),
                                               std::move(skinningDataExtractor),
                                               std::move(modelLoader),
                                               std::move(skeletonLoader));

      {
        modelConverter.load(as::ModelResources(gltfFile, skeletonFile));
        auto tritonModel = modelConverter.buildTritonModel();

        Log.info("Writing file with serialization version {0}", as::SERIAL_VERSION);

        if (outputFile.extension() == ".json") {
          if (std::ofstream outputStream(outputFile); outputStream) {
            cereal::JSONOutputArchive jsonOutput(outputStream);
            jsonOutput(tritonModel);
            Log.info("Wrote json output file to {0}", outputFile.string());
          } else {
            Log.error("Failed to open output file {0}", outputFile.string());
          }
        } else if (outputFile.extension() == ".trm") {
          if (std::ofstream outputStream(outputFile, std::ios::binary); outputStream) {
            cereal::PortableBinaryOutputArchive binOutput(outputStream);
            binOutput(tritonModel);
            outputStream.close();
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
