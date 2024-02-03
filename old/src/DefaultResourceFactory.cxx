#include "DefaultResourceFactory.hpp"

#include "Logger.hpp"

namespace Triton {
   DefaultResourceFactory::DefaultResourceFactory(std::filesystem::path rootPath) {
      Log::debug << "Constructed resource factory with rootPath: " << rootPath.string()
                 << std::endl;
   }

   DefaultResourceFactory::~DefaultResourceFactory() = default;

   std::string DefaultResourceFactory::createMesh(const std::string_view& filename) {
      Log::debug << "CreateMesh, filename: " << filename << std::endl;
      return "hello";
   }

   uint32_t DefaultResourceFactory::createTexture(const std::string_view& filename) {
      Log::debug << "createTexture, filename: " << filename << std::endl;
      return 1337;
   }
}
