#include "tr/SdfGenerator.hpp"
#include "tr/PlaneGenerator.hpp"
#include "tr/BoxGenerator.hpp"

namespace tr {

auto SdfGenerator::registerGenerator(SdfCreateInfo createInfo) -> GeneratorHandle {
  const auto key = keyGen.getKey();

  if (createInfo.shapeType == ShapeType::Plane) {
    const auto info = createInfo.get<PlaneInfo>();
    generatorMap.emplace(key, std::make_shared<PlaneGenerator>(info.normal, info.height));
  }

  if (createInfo.shapeType == ShapeType::Box) {
    const auto info = createInfo.get<BoxInfo>();
    generatorMap.emplace(key, std::make_shared<BoxGenerator>(info.center, info.size));
  }

  return key;
}

auto SdfGenerator::getGenerator(GeneratorHandle handle) -> std::shared_ptr<IDensityGenerator> {
  return generatorMap.at(handle);
}

}
