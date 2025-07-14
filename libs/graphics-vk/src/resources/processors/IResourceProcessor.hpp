#pragma once

#include "ProcessingResult.hpp"
#include "StagingRequirements.hpp"

namespace tr {

class IResourceProcessor {
public:
  IResourceProcessor() = default;
  virtual ~IResourceProcessor() = default;

  IResourceProcessor(const IResourceProcessor&) = default;
  IResourceProcessor(IResourceProcessor&&) = delete;
  auto operator=(const IResourceProcessor&) -> IResourceProcessor& = default;
  auto operator=(IResourceProcessor&&) -> IResourceProcessor& = delete;

  [[nodiscard]] virtual auto handles(std::type_index) const -> bool = 0;
  virtual auto analyze(std::shared_ptr<void> request) -> StagingRequirements = 0;
  virtual auto process(std::shared_ptr<void> request) -> ProcessingResult = 0;
};

}
