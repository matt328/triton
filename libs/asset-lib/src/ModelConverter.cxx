#include "as/ModelConverter.hpp"

#include "as/ConverterComponents.hpp"

#include "as/Model.hpp"

#include "as/gltf/GltfLoaderImpl.hpp"
#include "as/gltf/GltfNodeParser.hpp"

namespace as {
ModelConverter::ModelConverter(std::unique_ptr<ITransformParser> transformParser,
                               std::unique_ptr<IGeometryExtractor> geometryExtractor,
                               std::unique_ptr<ITextureExtractor> textureExtractor,
                               std::unique_ptr<ISkinningDataExtractor> skinningDataExtractor,
                               std::unique_ptr<IModelLoader<tinygltf::Model>> modelLoader,
                               std::unique_ptr<ISkeletonLoader> skeletonLoader)
    : skinningDataExtractor{std::move(skinningDataExtractor)},
      modelLoader{std::move(modelLoader)},
      skeletonLoader{std::move(skeletonLoader)} {
  nodeParser = std::make_unique<GltfNodeParser>(std::move(transformParser),
                                                std::move(geometryExtractor),
                                                std::move(textureExtractor));
}

auto ModelConverter::load(const std::filesystem::path& path) -> void {
  std::unique_ptr<IFileLoader<tinygltf::Model>> loader = std::make_unique<GltfLoaderImpl>();
  model = modelLoader->load(loader.get(), path);
}

auto ModelConverter::hasSkin() const -> bool {
  return !model.skins.empty();
}

auto ModelConverter::addSkeleton(const std::filesystem::path& path) -> void {
  skeleton = skeletonLoader->load(path);
}

void ModelConverter::load(const ModelResources& resources) {

  load(resources.modelPath);

  if (resources.skeletonPath.has_value()) {
    addSkeleton(*resources.skeletonPath);
  }
}

auto ModelConverter::isReady() const -> bool {
  const auto modelLoaded = !model.accessors.empty();
  const auto modelHasSkin = !model.skins.empty();
  const auto skeletonLoaded = skeleton.has_value();

  const auto skinlessModelLoaded = modelLoaded && !modelHasSkin;
  const auto skinnedModelLoaded = modelLoaded && modelHasSkin && skeletonLoaded;

  return skinlessModelLoaded || skinnedModelLoaded;
}

auto ModelConverter::buildTritonModel() const -> Model {

  auto tritonModel = Model{};

  for (const auto& scene = model.scenes[model.defaultScene]; const auto& nodeIndex : scene.nodes) {
    nodeParser->execute(model, model.nodes[nodeIndex], tritonModel);
  }

  if (skeleton.has_value()) {
    skinningDataExtractor->execute(model, *skeleton, tritonModel);
  }

  return tritonModel;
}
}
