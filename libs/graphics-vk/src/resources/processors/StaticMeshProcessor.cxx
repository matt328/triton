#include "StaticMeshProcessor.hpp"

namespace tr {

StaticMeshProcessor::StaticMeshProcessor(std::shared_ptr<TransferSystem> newTransferSystem)
    : transferSystem{std::move(newTransferSystem)} {
}

auto StaticMeshProcessor::handles(std::type_index eventType) const -> bool {
  return thisType == eventType;
}

auto StaticMeshProcessor::analyze(uint64_t batchId, std::shared_ptr<void> request)
    -> StagingRequirements {
  auto smRequest = std::static_pointer_cast<StaticMeshRequest>(request);
  const auto cargo = Cargo{
      .batchId = batchId,
      .requestId = smRequest->requestId,
      .entityName = smRequest->entityName,
  };
  return {.cargo = cargo, .responseType = typeid(StaticMeshUploaded), .geometryData = nullptr};
}

}
