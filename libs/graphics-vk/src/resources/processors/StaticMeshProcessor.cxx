#include "StaticMeshProcessor.hpp"

namespace tr {

StaticMeshProcessor::StaticMeshProcessor(std::shared_ptr<TransferSystem> newTransferSystem)
    : transferSystem{std::move(newTransferSystem)} {
}

auto StaticMeshProcessor::handles(std::type_index eventType) const -> bool {
  return thisType == eventType;
}

auto StaticMeshProcessor::process(std::shared_ptr<void> request) -> ProcessingResult {
  return {};
}

}
