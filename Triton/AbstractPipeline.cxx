#include "pch.hpp"

#include "AbstractPipeline.hpp"
#include "Utils.hpp"

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
   throw std::runtime_error(std::format("Failed to read shader from file {}", filename.data()));
}
