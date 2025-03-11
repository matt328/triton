#include "tr/SdfGenerator.hpp"
#include "tr/PlaneGenerator.hpp"

namespace tr {

auto SdfGenerator::registerGenerator(SdfCreateInfo createInfo) -> GeneratorHandle {
  const auto key = keyGen.getKey();
  if (createInfo.shapeType == ShapeType::Plane) {
    const auto info = createInfo.get<PlaneInfo>();
    generatorMap.emplace(key, std::make_shared<PlaneGenerator>(info.normal, info.height));
  }
  return key;
}

auto SdfGenerator::getGenerator(GeneratorHandle handle) -> std::shared_ptr<IDensityGenerator> {
  return generatorMap.at(handle);
}

}
