#pragma once

#include "IResourceProcessorFactory.hpp"

namespace tr {

class ResourceProcessorFactory : public IResourceProcessorFactory {
public:
  explicit ResourceProcessorFactory(std::vector<std::shared_ptr<IResourceProcessor>> processors);
  ~ResourceProcessorFactory() override = default;

  ResourceProcessorFactory(const ResourceProcessorFactory&) = default;
  ResourceProcessorFactory(ResourceProcessorFactory&&) = delete;
  auto operator=(const ResourceProcessorFactory&) -> ResourceProcessorFactory& = default;
  auto operator=(ResourceProcessorFactory&&) -> ResourceProcessorFactory& = delete;

  auto getProcessorFor(std::type_index type) -> std::shared_ptr<IResourceProcessor> override;

private:
  std::unordered_map<std::type_index, std::shared_ptr<IResourceProcessor>> processorMap;
};

}
