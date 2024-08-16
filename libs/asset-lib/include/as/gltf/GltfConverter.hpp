#pragma once

#include "Model.hpp"

namespace tinygltf {
   struct Primitive;
   class Model;
   class Node;
}
namespace tr::as::gltf {

   class TransformParser {
    public:
      virtual ~TransformParser() = default;
      [[nodiscard]] virtual glm::mat4 execute(const tinygltf::Node& node) const = 0;
   };

   class GeometryExtractor {
    public:
      virtual ~GeometryExtractor() = default;
      virtual void execute(const tinygltf::Model& model,
                           const tinygltf::Primitive& primitive,
                           const glm::mat4& transform,
                           Model& tritonModel) = 0;
   };

   class TextureExtractor {
    public:
      virtual ~TextureExtractor() = default;
      virtual void execute(const tinygltf::Model& model, int textureIndex, Model& tritonModel) = 0;
   };

   class SkinningDataExtractor {
    public:
      virtual ~SkinningDataExtractor() = default;
      virtual void execute(const tinygltf::Model& model, Model& tritonModel) = 0;
   };

   class ModelConverter {
    public:
      ModelConverter(std::unique_ptr<TransformParser> transformParser,
                     std::unique_ptr<GeometryExtractor> geometryExtractor,
                     std::unique_ptr<TextureExtractor> textureExtractor,
                     std::unique_ptr<SkinningDataExtractor> skinningDataExtractor)
          : transformParser{std::move(transformParser)},
            geometryExtractor{std::move(geometryExtractor)},
            textureExtractor{std::move(textureExtractor)},
            skinningDataExtractor{std::move(skinningDataExtractor)} {
      }

      auto convert(const std::filesystem::path& model, const std::filesystem::path& skeleton)
          -> Model;

    private:
      std::unique_ptr<TransformParser> transformParser;
      std::unique_ptr<GeometryExtractor> geometryExtractor;
      std::unique_ptr<TextureExtractor> textureExtractor;
      std::unique_ptr<SkinningDataExtractor> skinningDataExtractor;
   };

   class Converter {
    public:
      Converter() = default;
      ~Converter() noexcept;

      Converter(const Converter&) = delete;
      Converter& operator=(const Converter&) = delete;

      Converter(Converter&&) = delete;
      Converter& operator=(Converter&&) = delete;

      /// Reads a gltf file and a skeleton in order to build the runtime structure representing an
      /// animated model.  The skeleton is only referenced in order to map the joints in the model
      /// to those in the skeleton.
      static auto convert(const std::filesystem::path& gltf, const std::filesystem::path& skeleton)
          -> Model;
   };
}