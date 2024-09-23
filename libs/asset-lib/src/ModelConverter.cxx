#include "as/ModelConverter.hpp"

#include "as/ConverterComponents.hpp"

#include "as/Model.hpp"

#include "as/gltf/GltfLoaderImpl.hpp"
#include "as/gltf/GltfNodeParser.hpp"

namespace tr::as {
   ModelConverter::ModelConverter(std::unique_ptr<TransformParser> transformParser,
                                  std::unique_ptr<GeometryExtractor> geometryExtractor,
                                  std::unique_ptr<TextureExtractor> textureExtractor,
                                  std::unique_ptr<SkinningDataExtractor> skinningDataExtractor,
                                  std::unique_ptr<ModelLoader> modelLoader,
                                  std::unique_ptr<SkeletonLoader> skeletonLoader)
       : skinningDataExtractor{std::move(skinningDataExtractor)},
         modelLoader{std::move(modelLoader)},
         skeletonLoader{std::move(skeletonLoader)} {
      nodeParser = std::make_unique<gltf::GltfNodeParser>(std::move(transformParser),
                                                          std::move(geometryExtractor),
                                                          std::move(textureExtractor));
   }
   void ModelConverter::load(const ModelResources& resources) {

      std::unique_ptr<GltfFileLoader> loader = std::make_unique<gltf::GltfLoaderImpl>();

      model = modelLoader->load(loader.get(), resources.modelPath);
      skeleton = skeletonLoader->load(resources.skeletonPath);
   }

   auto ModelConverter::buildTritonModel() const -> Model {

      auto tritonModel = Model{};

      for (const auto& scene = model.scenes[model.defaultScene];
           const auto& nodeIndex : scene.nodes) {
         nodeParser->execute(model, model.nodes[nodeIndex], tritonModel);
      }

      skinningDataExtractor->execute(model, skeleton, tritonModel);

      return tritonModel;
   }
} // namespace tr::as
