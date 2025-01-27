#include "as/ConverterComponents.hpp"
#include "as/Model.hpp"

#include "as/ModelConverter.hpp"
#include "as/gltf/GltfGeometryExtractor.hpp"
#include "as/gltf/GltfModelLoader.hpp"
#include "as/gltf/GltfSkeletonLoader.hpp"
#include "as/gltf/GltfSkinningDataExtractor.hpp"
#include "as/gltf/GltfTextureExtractor.hpp"
#include "as/gltf/GltfTransformParser.hpp"
#include "GlmCereal.hpp"

#include <di.hpp>

namespace di = boost::di;

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

auto parseConverterOptions(const std::vector<std::string>& args) {
  namespace fs = std::filesystem;
  auto options = parseCommandLine(args);
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

  return as::ModelResources{.modelPath = gltfFile,
                            .skeletonPath = skeletonFile,
                            .outputFile = outputFile};
}

auto writeOutputFile(const std::filesystem::path& outputFile,
                     const as::Model& tritonModel) -> bool {
  if (outputFile.extension() == ".json") {
    std::ofstream outputStream(outputFile);
    if (!outputStream) {
      return false;
    }

    cereal::JSONOutputArchive jsonOutput(outputStream);
    jsonOutput(tritonModel);
    Log.info("Wrote json output file to {0}", outputFile.string());
    return true;
  }

  if (outputFile.extension() == ".trm") {
    std::ofstream outputStream(outputFile, std::ios::binary);
    if (!outputStream) {
      return false;
    }

    cereal::PortableBinaryOutputArchive binOutput(outputStream);
    binOutput(tritonModel);
    outputStream.close();
    Log.info("Wrote binary output file to {0}", outputFile.string());
    return true;
  }

  Log.error("Unsupported file extension: {0}", outputFile.extension().string());
  return false;
}

auto main(int argc, char* argv[]) -> int {
  initLogger(spdlog::level::trace, spdlog::level::trace);

  LogTest();

  std::vector<std::string> args(argv, argv + argc);

  auto converterOptions = parseConverterOptions(args);

  try {

    const auto injector =
        di::make_injector(di::bind<as::ITransformParser>.to<as::GltfTransformParser>(),
                          di::bind<as::IGeometryExtractor>.to<as::GltfGeometryExtractor>(),
                          di::bind<as::ITextureExtractor>.to<as::GltfTextureExtractor>(),
                          di::bind<as::ISkinningDataExtractor>.to<as::GltfSkinningDataExtractor>(),
                          di::bind<as::IModelLoader<tinygltf::Model>>.to<as::GltfModelLoader>(),
                          di::bind<as::ISkeletonLoader>.to<as::GltfSkeletonLoader>());

    auto modelConverter = injector.create<std::shared_ptr<as::ModelConverter>>();

    modelConverter->load(converterOptions);
    auto tritonModel = modelConverter->buildTritonModel();

    Log.info("Writing file with serialization version {0}", as::SERIAL_VERSION);

    if (!writeOutputFile(converterOptions.outputFile, tritonModel)) {
      Log.error("Failed to open output file {0}", converterOptions.outputFile.string());
      return -1;
    }

  } catch (const std::exception& ex) {
    Log.error(ex.what());
    return -1;
  }
  return 0;
}
