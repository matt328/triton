#include "ResourceProcessorFactory.hpp"
#include "api/fx/ResourceEvents.hpp"
#include "resources/processors/IResourceProcessor.hpp"

namespace tr {

ResourceProcessorFactory::ResourceProcessorFactory(
    std::vector<std::shared_ptr<IResourceProcessor>> processors) {
  for (const auto& p : processors) {
    if (p->handles(typeid(StaticModelRequest))) {
      processorMap[typeid(StaticModelRequest)] = p;
    }
    if (p->handles(typeid(StaticMeshRequest))) {
      processorMap[typeid(StaticMeshRequest)] = p;
    }
  }
}

auto ResourceProcessorFactory::getProcessorFor(std::type_index type)
    -> std::shared_ptr<IResourceProcessor> {
  assert(processorMap.contains(type));
  return processorMap.at(type);
}

}
