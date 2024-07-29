#pragma once

class NodeNoTransforms : public tinygltf::Node {
 public:
   std::vector<double> matrix = {};
   std::vector<double> translation = {};
   std::vector<double> rotation = {};
   std::vector<double> scale = {};
};