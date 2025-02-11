#include "SpirvShaderModuleFactory.hpp"

namespace tr {

SpirvShaderModuleFactory::SpirvShaderModuleFactory(std::shared_ptr<Device> newDevice)
    : device{std::move(newDevice)} {
}

SpirvShaderModuleFactory::~SpirvShaderModuleFactory() {
}

auto SpirvShaderModuleFactory::createShaderModule(
    [[maybe_unused]] vk::ShaderStageFlagBits shaderType,
    const std::filesystem::path& filename) const -> vk::raii::ShaderModule {
  const auto spirv = readSPIRVFile(filename.string());
  const auto shaderCreateInfo =
      vk::ShaderModuleCreateInfo{.codeSize = 4 * spirv.size(), .pCode = spirv.data()};

  return device->getVkDevice().createShaderModule(shaderCreateInfo);
}

auto SpirvShaderModuleFactory::readSPIRVFile(const std::string& filename) -> std::vector<uint32_t> {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (!file) {
    throw std::runtime_error("Failed to open SPIR-V file: " + filename);
  }

  // Get file size in bytes
  std::streamsize fileSize = file.tellg();

  if (fileSize % sizeof(uint32_t) != 0) {
    throw std::runtime_error("SPIR-V file size is not a multiple of 4 bytes");
  }

  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

  // Seek back to the beginning and read into the vector
  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

  return buffer;
}

}
