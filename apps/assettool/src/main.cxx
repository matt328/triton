#include "GltfConverter.hpp"
#include "GlmCereal.hpp"
#include <filesystem>

#include <nfd.hpp>

auto parseCommandLine(int argc, char* argv[]) {
   auto options = std::unordered_map<std::string, std::string>{};

   NFD::Guard nfdGuard;

   auto outPath = NFD::UniquePath{};

   auto filterItems = std::array<nfdfilteritem_t, 2>{nfdfilteritem_t{"Source code", "c,cpp,cc,cxx"},
                                                     nfdfilteritem_t{"Headers", "h,hpp"}};

   // show the dialog
   const auto result = NFD::OpenDialog(outPath, filterItems.data(), filterItems.size());

   if (result == NFD_OKAY) {
      Log::info << "Success: " << outPath.get() << std::endl;
   } else if (result == NFD_CANCEL) {
      Log::info << "User pressed cancel." << std::endl;
   } else {
      Log::error << "Error: " << NFD::GetError() << std::endl;
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
   namespace fs = std::filesystem;
   Log::LogManager::getInstance().setMinLevel(Log::Level::Trace);

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

      Log::info << "Converting Gltf file, input: " << gltfFile.string()
                << ", skeletonFile: " << skeletonFile.string() << std::endl;

      auto converter = al::gltf::Converter{};

      try {
         auto tritonModel = converter.convert(gltfFile, skeletonFile);
         {
            auto os = std::ofstream(outputFile, std::ios::binary);
            cereal::BinaryOutputArchive output(os);
            output(tritonModel);
         }
         Log::info << "Wrote binary output file to " << outputFile.string() << std::endl;
      } catch (const std::exception& ex) { Log::error << ex.what() << std::endl; }
   } else {
      Log::error << "First arg must be 'gltf' for now" << std::endl;
      return -1;
   }

   return 0;
}
