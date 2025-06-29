#pragma once

#include "api/action/Actions.hpp"
#include "api/action/Inputs.hpp"
#include "api/fx/EditorEvents.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "api/gw/GameplayEvents.hpp"
#include "api/gw/TransformData.hpp"
#include "api/vtx/SdfCreateInfo.hpp"
#include "api/vtx/TerrainResult.hpp"
#include "bk/Color.hpp"
#include "bk/Handle.hpp"

#include "ResourceEvents.hpp"

namespace tr {

struct BoxWidget {
  std::string tag;
  glm::vec3 center;
  float extent;
  Color color{Colors::White};
  std::optional<GameObjectId> target{std::nullopt};
  std::optional<glm::vec3> targetOffset{std::nullopt};
};

struct DeleteObject {
  GameObjectId objectId;
};

struct TransformObject {
  GameObjectId objectId;
  TransformData transformData;
};

struct ChunkTriangulateRequest {
  GameObjectId terrainId;
  GameObjectId chunkId;
  glm::ivec3 cellPosition;
};

struct TerrainCreateRequest {
  uint64_t requestId;
  std::string name;
  SdfCreateInfo sdfCreateInfo;
  glm::ivec3 chunkCount; // Dimensions of the terrain, in chunks
  glm::ivec3 chunkSize;  // Number of points in the chunk, number of cells is chunkSize.[x|y|z] - 2
};

struct TerrainCreated {
  uint64_t requestId;
  std::string name;
  TerrainHandle terrainHandle;
  SdfHandle sdfHandle;
  std::vector<BlockResult> chunks;
  glm::vec3 terrainSize;
  std::optional<tr::GameObjectId> entityId = std::nullopt;
};

struct WindowIconified {
  int iconified;
};

struct WindowClosed {};

struct KeyEvent {
  tr::Key key;
  tr::ButtonState buttonState;
};

struct MouseMoved {
  double x;
  double y;
};

struct Fullscreen {
  bool isFullscreen;
};

struct MouseCaptured {
  bool isMouseCaptured;
};

struct MouseButton {
  int button;
  int action;
  int mods;
};

struct PlayerMoved {
  int playerId;
  float x, y;
};

struct PlayerScored {
  int playerId;
  int score;
};

struct SwapchainResized {
  uint32_t width;
  uint32_t height;
};

struct SwapchainCreated {
  uint32_t width;
  uint32_t height;
};

struct FrameEndEvent {
  // This is nasty, but it works for now
  std::any fenceHandle;
};

using EventVariant = std::variant<AddStaticModel,
                                  AddSkeleton,
                                  AddAnimation,
                                  AddModel,
                                  SelectEntity,
                                  SaveProject,
                                  LoadProject,
                                  BoxWidget,
                                  DeleteObject,
                                  TransformObject,
                                  ChunkTriangulateRequest,
                                  TerrainCreateRequest,
                                  TerrainCreated,
                                  DynamicModelResponse,
                                  DynamicModelRequest,
                                  CreateStaticGameObject,
                                  StaticMeshRequest,
                                  UploadGeometryRequest,
                                  UploadGeometryResponse,
                                  StaticModelRequest,
                                  StaticModelUploaded,
                                  UploadImageRequest,
                                  UploadImageResponse,
                                  WindowIconified,
                                  WindowClosed,
                                  KeyEvent,
                                  MouseMoved,
                                  MouseButton,
                                  Fullscreen,
                                  MouseCaptured,
                                  Action,
                                  PlayerMoved,
                                  PlayerScored,
                                  EntityCreated,
                                  SwapchainResized,
                                  SwapchainCreated,
                                  FrameEndEvent,
                                  BeginResourceBatch,
                                  EndResourceBatch>;
}
