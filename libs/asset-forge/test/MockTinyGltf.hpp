#pragma once

#include "tiny_gltf.h"

class NodeNoTransforms : public tinygltf::Node {};

class TranslationTransforms : public tinygltf::Node {
public:
  TranslationTransforms() {
    translation = std::vector<double>{1, 1, 1};
  }
};

class RotationTransforms : public tinygltf::Node {
public:
  RotationTransforms() {
    rotation = std::vector<double>{0.7, 0.7, 0, 0};
  }
};

class ScaleTransforms : public tinygltf::Node {
public:
  ScaleTransforms() {
    scale = std::vector<double>{0.7, 0.7, 0.7};
  }
};

class MatrixTransforms : public tinygltf::Node {
public:
  MatrixTransforms() {
    matrix = std::vector<double>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  }
};
