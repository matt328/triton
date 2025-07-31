#pragma once

namespace tr {

class IResourceProcessor;

class IResourceProcessorFactory {
public:
  IResourceProcessorFactory() = default;
  virtual ~IResourceProcessorFactory() = default;

  IResourceProcessorFactory(const IResourceProcessorFactory&) = default;
  IResourceProcessorFactory(IResourceProcessorFactory&&) = delete;
  auto operator=(const IResourceProcessorFactory&) -> IResourceProcessorFactory& = default;
  auto operator=(IResourceProcessorFactory&&) -> IResourceProcessorFactory& = delete;

  virtual auto getProcessorFor(std::type_index type) -> std::shared_ptr<IResourceProcessor> = 0;
};

}
