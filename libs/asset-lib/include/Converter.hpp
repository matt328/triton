#pragma once

#include "ModelData.hpp"
#include <set>

namespace al {
   class Converter {
    public:
      Converter() noexcept = default;
      ~Converter() noexcept;

      Converter(const Converter&) = delete;
      Converter& operator=(const Converter&) = delete;

      Converter(Converter&&) = delete;
      Converter& operator=(Converter&&) = delete;

      auto convertFbx(const std::filesystem::path& filePath) -> ModelData;

      void traverseNode(FbxNode* node, std::vector<Model>& models);
      void processMesh(FbxNode* node, std::vector<Model>& models);

      void extractFbxJoints(FbxNode* node,
                            std::unordered_map<std::string, int>& fbxJoints,
                            int& index);

      auto jointInfluencesVertices(FbxNode* jointNode, std::vector<FbxNode*> meshNodes) -> bool;

      void checkInfluences(FbxNode* node,
                           std::vector<FbxNode*> meshNodes,
                           std::set<FbxNode*>& influencingJoints);

      auto createJointMapping(const std::unordered_map<std::string, int>& fbxJoints,
                              const std::unordered_map<std::string, int>& ozzJoints)
          -> std::unordered_map<int, int>;

      void collectMeshNodes(FbxNode* node, std::vector<FbxNode*>& meshNodes);

      // helper functions
      // Generic function that gets an element from a layer.
      template <typename _Element>
      bool getElement(const _Element& layer,
                      int vertexId,
                      int controlPoint,
                      typename _Element::ArrayElementType* out) {
         assert(out);

         int direct_array_id{};
         switch (layer.GetMappingMode()) {
            case FbxGeometryElement::eByControlPoint: {
               switch (layer.GetReferenceMode()) {
                  case FbxGeometryElement::eDirect: {
                     direct_array_id = controlPoint;
                     break;
                  }
                  case FbxGeometryElement::eIndexToDirect: {
                     direct_array_id = layer.GetIndexArray().GetAt(controlPoint);
                     break;
                  }
                  default:
                     return false; // Unhandled reference mode.
               }
               break;
            }
            case FbxGeometryElement::eByPolygonVertex: {
               switch (layer.GetReferenceMode()) {
                  case FbxGeometryElement::eDirect: {
                     direct_array_id = vertexId;
                     break;
                  }
                  case FbxGeometryElement::eIndexToDirect: {
                     direct_array_id = layer.GetIndexArray().GetAt(vertexId);
                     break;
                  }
                  default:
                     return false; // Unhandled reference mode.
               }
               break;
            }
            default:
               return false; // Unhandled mapping mode.
         }

         // Extract data from the layer direct array.
         *out = layer.GetDirectArray().GetAt(direct_array_id);

         return true;
      }
   };
}