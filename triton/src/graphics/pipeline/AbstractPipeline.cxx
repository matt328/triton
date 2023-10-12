#include "AbstractPipeline.hpp"
#include "core/Utils.hpp"

namespace Triton {
   std::string AbstractPipeline::readShaderFile(const std::string_view& filename) const {
      if (std::ifstream file(filename.data(), std::ios::binary); file.is_open()) {
         file.seekg(0, std::ios::end);

         const std::streampos fileSize = file.tellg();

         file.seekg(0, std::ios::beg);

         std::string shaderCode;
         shaderCode.resize(fileSize);
         file.read(shaderCode.data(), fileSize);

         file.close();
         return shaderCode;
      }
      std::stringstream ss;
      ss << "Failed to read shader from file " << filename.data();
      throw std::runtime_error(ss.str());
   }
}