#include "GltfConverter.hpp"
#include "GlmCereal.hpp"

auto parseCommandLine(int argc, char* argv[]) {
   auto options = std::unordered_map<std::string, std::string>{};

   NFD::Guard nfdGuard;

   auto outPath = NFD::UniquePath{};

   auto filterItems = std::array<nfdfilteritem_t, 1>{nfdfilteritem_t{"gltf Model", "gltf, glb"}};

   // show the dialog
   const auto result = NFD::OpenDialog(outPath, filterItems.data(), filterItems.size());

   if (result == NFD_OKAY) {
      Log.info("Success: {0}", outPath.get());
   } else {
      Log.error("Error: {0}", NFD::GetError());
   }

   options["mode"] = argv[1];

   for (int i = 2; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg.starts_with('-')) {
         auto delimiter_pos = arg.find('=');
         if (delimiter_pos != std::string::npos) {
            std::string key = arg.substr(1, delimiter_pos - 1); // Skip the leading '-'
            std::string value = arg.substr(delimiter_pos + 1);
            options[key] = value;
         }
      }
   }

   return options;
}

int main(int argc, char* argv[]) {

   initLogger();
   Log.set_level(spdlog::level::trace);

   namespace fs = std::filesystem;
   auto options = parseCommandLine(argc, argv);

   if (options["mode"] == "gltf") {
      const auto gltfFileStr = options["f"];
      auto gltfFile = fs::path{gltfFileStr};

      if (gltfFile.is_relative()) {
         gltfFile = fs::absolute(gltfFile);
      }

      auto skeletonFile = fs::path{options["s"]};
      if (skeletonFile.is_relative()) {
         skeletonFile = fs::absolute(skeletonFile);
      }

      auto outputFile = fs::path{options["o"]};
      if (outputFile.is_relative()) {
         outputFile = fs::absolute(outputFile);
      }

      Log.info("Converting Gltf file, input: {0}, skeleton file: {1}",
               gltfFile.string(),
               skeletonFile.string());

      auto converter = al::gltf::Converter{};

      try {
         auto tritonModel = converter.convert(gltfFile, skeletonFile);
         {
            auto os = std::ofstream(outputFile, std::ios::binary);
            cereal::BinaryOutputArchive output(os);
            output(tritonModel);
         }
         Log.info("Wrote binary output file to {0}", outputFile.string());
      } catch (const std::exception& ex) { Log.error(ex.what()); }
   } else {
      Log.error("First arg must be 'gltf' for now");
      return -1;
   }

   return 0;
}
