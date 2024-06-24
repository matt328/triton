#include "Converter.hpp"
#include "Model.hpp"
#include <fbxsdk/scene/geometry/fbxdeformer.h>
#include <fbxsdk/scene/geometry/fbxlayer.h>
#include <fbxsdk/scene/geometry/fbxnodeattribute.h>
#include <fbxsdk/utils/fbxgeometryconverter.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/animation_optimizer.h>
#include <ozz/animation/offline/fbx/fbx_animation.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/base/endianness.h>
#include <ozz/base/maths/soa_float.h>
#include <span>
#include <ozz/animation/runtime/skeleton_utils.h>
#include <string>
#include <unordered_map>

namespace al {

   using namespace ozz::animation::offline;

   Converter::~Converter() noexcept {
   }

   auto Converter::convertFbx(const std::filesystem::path& filePath) -> ModelData {
      auto mgr = fbx::FbxManagerInstance();
      auto loader = fbx::FbxSceneLoader{filePath.string().c_str(),
                                        nullptr,
                                        mgr,
                                        fbx::FbxDefaultIOSettings{mgr}};

      if (!loader.scene()) {
         Log::error << "Failed to load scene" << std::endl;
      }

      // Parse Skeleton
      auto rawSkeleton = RawSkeleton{};
      OzzImporter::NodeType types = {.skeleton = true};
      auto result = fbx::ExtractSkeleton(loader, types, &rawSkeleton);
      if (!result) {
         Log::error << "failed to extract skeleton" << std::endl;
      }

      auto builtSkeleton = SkeletonBuilder{}(rawSkeleton);

      for (const auto& name : builtSkeleton->joint_names()) {
         Log::debug << "fbx skeleton joint: " << name << std::endl;
      }

      // Parse Animations
      const auto animationNames = fbx::GetAnimationNames(loader);
      const auto animationName = animationNames[0];

      // Get frame rate from the scene.
      FbxTime::EMode mode = loader.scene()->GetGlobalSettings().GetTimeMode();
      const auto frameRate = static_cast<float>(
          (mode == FbxTime::eCustom) ? loader.scene()->GetGlobalSettings().GetCustomFrameRate()
                                     : FbxTime::GetFrameRate(mode));

      auto rawAnimation = RawAnimation{};
      rawAnimation.name = animationName;
      auto optimizedAnimation = RawAnimation{};
      optimizedAnimation.name = animationName;
      const auto extractionResult = fbx::ExtractAnimation(animationName.c_str(),
                                                          loader,
                                                          *builtSkeleton,
                                                          frameRate,
                                                          &rawAnimation);

      if (!extractionResult) {
         Log::error << "Error extracting animation" << std::endl;
      }

      const auto optimizationResult =
          AnimationOptimizer{}(rawAnimation, *builtSkeleton, &optimizedAnimation);

      auto animation = AnimationBuilder{}(optimizedAnimation);

      if (!optimizationResult) {
         Log::error << "error optimizing animation" << std::endl;
      }

      auto models = std::vector<Model>{};

      // Load geometry data
      FbxGeometryConverter converter(mgr);
      converter.RemoveBadPolygonsFromMeshes(loader.scene());
      if (!converter.Triangulate(loader.scene(), true)) {
         Log::error << "Failed to triangulate meshes" << std::endl;
      }

      traverseNode(loader.scene()->GetRootNode(), models);

      /*
         The skeletal data in the model contains extra joints that aren't actually used to transform
         any vertices.  However, the skeleton nor animation systems know this, so they calculate
         model matrices for them anyway. They have to in the case where they might have child bones
         that do transform vertices.  But the vertex skinning data for some reason uses only the
         list of joints that affect vertices when indexing into them with the joints vertex
         attribute. This is worth a post on SO asking for an explanation as to why there is so much
         complexity to work around this.

         Need to double check that the joints and weights I am extracting from the fbx file match
         those extracted by the fbx2mesh sample.

         [x] Anyway, we need to iterate over all the joints in the skeleton, in heirarchical order,
         and place each one's position in the master list in a map, excluding the joints that don't
         affect any vertices.

         [ ] Vertex joint indices and weights are all incorrect. Maybe.

         [ ] We also need to calculate and add the joints' inverse bind poses to a parallel list
         whose order mirrors that of the values of the jointRemaps map.

         Once those two pieces are added correctly, I think we should be ready to render.

         Need to do this incrementally.
         - Render the skeleton in bind pose.
         - Use the time slider to render the skeleton in sampled animation poses
         - Render the Mesh in bind pose with the skeleton underneath
         - List the joints in the ui, and select a joint to color it, and the vertices affected by
         it
         - Next transform the vertices along with the skeleton and render the mesh in sampled
         animation poses.
         - ???
         - Profit.
         - Clean up this god awful mesh processing stuff.
      */
      /*
         Parsing FBX files is a nightmare. Somehow even moreso than gltf.
         We might have more success going back to gltf.

         [ ] Compare skeletons generated from fbx vs gltf

      */

      auto fbxJoints = std::unordered_map<std::string, int>{};
      int index{};
      extractFbxJoints(loader.scene()->GetRootNode(), fbxJoints, index);

      std::vector<FbxNode*> meshNodes{};
      collectMeshNodes(loader.scene()->GetRootNode(), meshNodes);

      std::set<FbxNode*> influencingJoints{};
      checkInfluences(loader.scene()->GetRootNode(), meshNodes, influencingJoints);

      std::set<std::string> influencingJointNames{};
      for (const auto node : influencingJoints) {
         influencingJointNames.insert(node->GetName());
      }

      auto jointRemaps = std::unordered_map<int, int>{};
      int position = 0;

      ozz::animation::IterateJointsDF(
          *builtSkeleton,
          [&influencingJointNames, &builtSkeleton, &jointRemaps, &position](int current,
                                                                            int parent) {
             const auto& currentName = builtSkeleton->joint_names()[current];

             if (influencingJointNames.find(currentName) != influencingJointNames.end()) {
                jointRemaps[position] = current;
                ++position;
             }
          });

      models[0].jointRemaps = std::move(jointRemaps);

      return ModelData{.model = models[0],
                       .skeleton = std::move(*builtSkeleton),
                       .animation = std::move(*animation)};
   }

   void Converter::extractFbxJoints(FbxNode* node,
                                    std::unordered_map<std::string, int>& fbxJoints,
                                    int& index) {
      if (node->GetNodeAttribute() &&
          node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
         auto jointName = node->GetName();
         fbxJoints[jointName] = index++;
      }

      for (int i = 0; i < node->GetChildCount(); ++i) {
         extractFbxJoints(node->GetChild(i), fbxJoints, index);
      }
   }

   void Converter::collectMeshNodes(FbxNode* node, std::vector<FbxNode*>& meshNodes) {
      if (!node) {
         return;
      }

      if (node->GetNodeAttribute() &&
          node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
         meshNodes.push_back(node);
      }

      for (int i = 0; i < node->GetChildCount(); ++i) {
         collectMeshNodes(node->GetChild(i), meshNodes);
      }
   }

   auto Converter::jointInfluencesVertices(FbxNode* jointNode, std::vector<FbxNode*> meshNodes)
       -> bool {
      for (auto meshNode : meshNodes) {
         if (!meshNode || !meshNode->GetMesh()) {
            continue;
         }

         auto mesh = meshNode->GetMesh();
         auto skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);

         for (int i = 0; i < skinCount; ++i) {
            auto skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
            auto clusterCount = skin->GetClusterCount();

            for (int j = 0; j < clusterCount; ++j) {
               auto cluster = skin->GetCluster(j);
               if (cluster->GetLink() == jointNode) {
                  return true;
               }
            }
         }
      }

      return false;
   }

   void Converter::checkInfluences(FbxNode* node,
                                   std::vector<FbxNode*> meshNodes,
                                   std::set<FbxNode*>& influencingJoints) {
      if (!node) {
         return;
      }

      if (node->GetNodeAttribute() &&
          node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
         if (jointInfluencesVertices(node, meshNodes)) {
            influencingJoints.insert(node);
         }
      }

      for (int i = 0; i < node->GetChildCount(); ++i) {
         checkInfluences(node->GetChild(i), meshNodes, influencingJoints);
      }
   }

   auto Converter::createJointMapping(const std::unordered_map<std::string, int>& fbxJoints,
                                      const std::unordered_map<std::string, int>& ozzJoints)
       -> std::unordered_map<int, int> {
      std::unordered_map<int, int> jointMapping;

      for (const auto& fbxJoint : fbxJoints) {
         const auto& jointName = fbxJoint.first;
         if (ozzJoints.find(jointName) != ozzJoints.end()) {
            jointMapping[fbxJoint.second] = ozzJoints.at(jointName);
         } else {
            Log::warn << "Joint not found in ozz skeleton: " << jointName << std::endl;
         }
      }

      return jointMapping;
   }

   void Converter::traverseNode(FbxNode* node, std::vector<Model>& models) {
      if (node->GetNodeAttribute() &&
          node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
         processMesh(node, models);
      }

      int numChildren = node->GetChildCount();
      for (int i = 0; i < numChildren; i++) {
         traverseNode(node->GetChild(i), models);
      }
   }

   void Converter::processMesh(FbxNode* node, std::vector<Model>& models) {
      auto mesh = node->GetMesh();

      if (!mesh) {
         return;
      }

      // Vertices
      int numVertices = mesh->GetControlPointsCount();
      std::vector<Vertex> vertices(numVertices);
      for (int i = 0; i < numVertices; ++i) {
         vertices[i].position = glm::vec3{mesh->GetControlPointAt(i)[0],
                                          mesh->GetControlPointAt(i)[1],
                                          mesh->GetControlPointAt(i)[2]};
      }

      // UV Coordinates
      auto uvSetNameList = FbxStringList{};
      mesh->GetUVSetNames(uvSetNameList);
      const auto uvSetName = uvSetNameList.GetStringAt(0); // Assume only one set

      for (int i = 0; i < mesh->GetPolygonCount(); ++i) {
         for (int j = 0; j < mesh->GetPolygonSize(i); ++j) {
            auto ctrlPointIndex = mesh->GetPolygonVertex(i, j);
            auto uv = FbxVector2{};
            auto unmapped = bool{};
            mesh->GetPolygonVertexUV(i, j, uvSetName, uv, unmapped);
            vertices[ctrlPointIndex].uv = glm::vec2{uv[0], uv[1]};
         }
      }

      // Normals
      for (int i = 0; i < mesh->GetPolygonCount(); ++i) {
         for (int j = 0; j < mesh->GetPolygonSize(i); ++j) {
            auto ctrlPointIndex = mesh->GetPolygonVertex(i, j);
            FbxVector4 normal;
            mesh->GetPolygonVertexNormal(i, j, normal);
            vertices[ctrlPointIndex].normal = glm::vec3{normal[0], normal[1], normal[2]};
         }
      }

      Log::info << "vertex count: " << vertices.size() << std::endl;

      auto controlPointSkinData = std::map<int, std::vector<std::pair<int, double>>>{};

      // Skinning Information
      auto skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
      for (int i = 0; i < skinCount; ++i) {
         auto skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
         if (!skin) {
            continue;
         }
         auto clusterCount = skin->GetClusterCount();
         for (int j = 0; j < clusterCount; ++j) {
            auto cluster = skin->GetCluster(j);
            if (!cluster) {
               continue;
            }

            auto joint = cluster->GetLink();
            if (!joint) {
               continue;
            }

            auto jointIndex = j;
            auto controlPointIndices = cluster->GetControlPointIndices();
            auto controlPointWeights = cluster->GetControlPointWeights();
            auto contolPointIndicesCount = cluster->GetControlPointIndicesCount();

            for (int k = 0; k < contolPointIndicesCount; ++k) {
               auto vertexIndex = controlPointIndices[k];
               vertices[vertexIndex].joints = glm::zero<glm::uvec4>();
               vertices[vertexIndex].weights = glm::zero<glm::vec4>();
            }

            for (int k = 0; k < contolPointIndicesCount; ++k) {
               auto vertexIndex = controlPointIndices[k];
               auto weight = controlPointWeights[k];
               controlPointSkinData[vertexIndex].emplace_back(jointIndex, weight);
            }
         }
      }

      // Apply Skinning data to vertices with culling and normalization
      for (auto& [vertexIndex, skinData] : controlPointSkinData) {
         // Sort influences by weight in descending order
         std::sort(skinData.begin(), skinData.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
         });

         // Cull to the top 4 influences
         if (skinData.size() > 4) {
            skinData.resize(4);
         }

         // Normalize weights
         double totalWeight = 0.0;
         for (const auto& [jointIndex, weight] : skinData) {
            totalWeight += weight;
         }
         for (auto& [jointIndex, weight] : skinData) {
            weight /= totalWeight;
         }

         // Assign to the vertex
         int index = 0;
         for (const auto& [jointIndex, weight] : skinData) {
            vertices[vertexIndex].joints[index] = jointIndex;
            vertices[vertexIndex].weights[index] = static_cast<float>(weight);
            ++index;
         }
      }

      // Indices
      int numIndices = mesh->GetPolygonVertexCount();
      const uint32_t* rawIndices = reinterpret_cast<uint32_t*>(mesh->GetPolygonVertices());
      const auto sp = std::span<const uint32_t>(rawIndices, numIndices);
      std::vector<uint32_t> indices{sp.begin(), sp.end()};

      models.emplace_back(vertices,
                          indices,
                          std::unordered_map<int, int>{},
                          std::vector<glm::mat4>{});

      Log::info << "index count: " << indices.size() << std::endl;
   }
}
