#pragma once

#include "IShaderModuleFactory.hpp"
#include "vk/core/Device.hpp"

namespace tr {

class SpirvShaderModuleFactory : public IShaderModuleFactory {
public:
  explicit SpirvShaderModuleFactory(std::shared_ptr<Device> newDevice);
  ~SpirvShaderModuleFactory() override;

  SpirvShaderModuleFactory(const SpirvShaderModuleFactory&) = default;
  SpirvShaderModuleFactory(SpirvShaderModuleFactory&&) = delete;
  auto operator=(const SpirvShaderModuleFactory&) -> SpirvShaderModuleFactory& = default;
  auto operator=(SpirvShaderModuleFactory&&) -> SpirvShaderModuleFactory& = delete;

  [[nodiscard]] auto createShaderModule(vk::ShaderStageFlagBits shaderType,
                                        const std::filesystem::path& filename) const
      -> vk::raii::ShaderModule override;

private:
  std::shared_ptr<Device> device;

  [[nodiscard]] static auto readSPIRVFile(const std::string& filename) -> std::vector<uint32_t>;
};

}
