#pragma once

#include "IResourceProcessor.hpp"

namespace tr {

class TransferSystem;

class StaticMeshProcessor : public IResourceProcessor {
public:
  StaticMeshProcessor(std::shared_ptr<TransferSystem> newTransferSystem);
  ~StaticMeshProcessor() = default;

  StaticMeshProcessor(const StaticMeshProcessor&) = default;
  StaticMeshProcessor(StaticMeshProcessor&&) = delete;
  auto operator=(const StaticMeshProcessor&) -> StaticMeshProcessor& = default;
  auto operator=(StaticMeshProcessor&&) -> StaticMeshProcessor& = delete;

  [[nodiscard]] auto handles(std::type_index) const -> bool override;
  auto process(std::shared_ptr<void> request) -> ProcessingResult override;

private:
  std::shared_ptr<TransferSystem> transferSystem;

  std::type_index thisType = typeid(StaticMeshRequest);
};

}
