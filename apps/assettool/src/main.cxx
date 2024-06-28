#include "docopt.h"
#include "GltfConverter.hpp"

static const auto USAGE =
    R"(Asset Tool.

    Usage:
      assettool gltf --file <gltf>... --skeleton <skeleton>...

    Options:
      -h --help                             Show this screen.
      -v, --version                         Show version.
      -f <name>, --file <name>              A file
      -s <skeleton>, --skeleton <skeleton>  A skeleton
)";

int main(int argc, const char** argv) {
   Log::LogManager::getInstance().setMinLevel(Log::Level::Trace);

   auto args = docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "Asset Tool 0.0.1");

   if (args.find("gltf") != args.end() && args["gltf"].asBool() == true) {
      const auto gltfFile =
          std::filesystem::absolute(std::filesystem::path{args["--file"].asStringList()[0]});
      const auto skeletonFile =
          std::filesystem::absolute(std::filesystem::path{args["--skeleton"].asStringList()[0]});

      Log::info << "gltf branch, gltf file: " << gltfFile.string()
                << ", skeletonFile: " << skeletonFile.string() << std::endl;

      auto converter = al::gltf::Converter{};

      auto result = converter.convert(gltfFile, skeletonFile);
   }

   return 0;
}
