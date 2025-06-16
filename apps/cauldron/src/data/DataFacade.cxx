#include "DataFacade.hpp"

#include "api/fx/IEventQueue.hpp"

namespace ed {

// constexpr auto ModelFile =
//     "/home/matt/projects/matt/game-assets/models/current/viking_room/viking_room_v4.trm";

constexpr auto ModelFile =
    "/home/matt/Projects/game-assets/models/current/viking_room/viking_room_v4.trm";

DataFacade::DataFacade(std::shared_ptr<tr::IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
  Log.trace("Creating DataFacade");

  eventQueue->subscribe<tr::TerrainCreated>([this](const tr::TerrainCreated& event) {
    // dataStore.entityNameMap.emplace(event.name, event.entityId.value());

    auto chunkIds = std::vector<BlockData>{};
    chunkIds.reserve(event.chunks.size());
    std::ranges::transform(event.chunks.begin(),
                           event.chunks.end(),
                           std::back_inserter(chunkIds),
                           [](const tr::BlockResult& chunk) {
                             return BlockData{.entityId = chunk.entityId.value(),
                                              .location = chunk.location};
                           });

    std::ranges::sort(chunkIds, [](const BlockData& a, const BlockData& b) {
      return std::tie(a.location.z, a.location.y, a.location.x) <
             std::tie(b.location.z, b.location.y, b.location.x);
    });

    dataStore.terrainMap.emplace(event.name,
                                 TerrainData{.name = std::string{event.name},
                                             .terrainSize = event.terrainSize,
                                             .chunkData = chunkIds,
                                             .entityId = event.entityId.value()});

    // Register each Chunk's name -> id
    // for (const auto& chunk : event.chunks) {
    // dataStore.entityNameMap.emplace(chunk.name, chunk.entityId.value());
    // }

    engineBusy = false;
  });

  eventQueue->subscribe<tr::StaticModelResponse>(
      [&](const tr::StaticModelResponse& event) { handleStaticModelResponse(event); },
      "test_group");

  scheduleDelayed([this] { testResources(); }, std::chrono::seconds(1));
}

DataFacade::~DataFacade() {
  Log.trace("Destroying DataFacade");
}

void DataFacade::scheduleDelayed(std::function<void()> func, std::chrono::milliseconds delay) {
  std::thread([func = std::move(func), delay]() {
    std::this_thread::sleep_for(delay);
    func();
  }).detach(); // fire-and-forget
}

auto DataFacade::testResources() -> void {
  const auto beginBatch = tr::BeginResourceBatch{.batchId = 1};
  const auto endBatch = tr::EndResourceBatch{.batchId = 1};
  const auto vikingRoomRequestId = requestIdGenerator.getKey();
  const auto vikingRoomRequest = tr::StaticModelRequest{.batchId = 1,
                                                        .requestId = vikingRoomRequestId,
                                                        .modelFilename = ModelFile,
                                                        .entityName = "Viking Room #1"};
  const auto vikingRoomRequest2Id = requestIdGenerator.getKey();
  const auto vikingRoomRequest2 = tr::StaticModelRequest{.batchId = 1,
                                                         .requestId = vikingRoomRequest2Id,
                                                         .modelFilename = ModelFile,
                                                         .entityName = "Viking Room #2"};

  const auto peasantRequestId = requestIdGenerator.getKey();
  const auto peasant = tr::DynamicModelRequest{.batchId = 1,
                                               .requestId = peasantRequestId,
                                               .modelFilename = ModelFile,
                                               .entityName = "Viking Room #2"};

  inFlightMap.emplace(vikingRoomRequestId,
                      EntityData{.name = "viking room 1",
                                 .orientation = Orientation{.position = glm::vec3{1.f, 2.4f, 3.f}},
                                 .modelName = "VikingRoom",
                                 .skeleton = "",
                                 .animations = {}});

  inFlightMap.emplace(vikingRoomRequest2Id,
                      EntityData{.name = "Viking Room 2",
                                 .orientation = Orientation{.position = glm::vec3{1.f, 2.4f, 3.f}},
                                 .modelName = "VikingRoom",
                                 .skeleton = "",
                                 .animations = {}});
  inFlightMap.emplace(peasantRequestId,
                      EntityData{.name = "peasant1",
                                 .orientation = Orientation{.position = glm::vec3{1.f, 2.4f, 3.f}},
                                 .modelName = "PeasantModel",
                                 .skeleton = "MainSkeleton",
                                 .animations = {"Idle", "Walk"}});

  eventQueue->emit(beginBatch, "test_group");
  eventQueue->emit(vikingRoomRequest, "test_group");
  eventQueue->emit(peasant, "test_group");
  eventQueue->emit(vikingRoomRequest2, "test_group");
  eventQueue->emit(endBatch, "test_group");
}

auto DataFacade::handleStaticModelResponse(const tr::StaticModelResponse& response) -> void {
  Log.trace("Received StaticModelResponse id={}, gameObjectId={}",
            response.requestId,
            response.gameObjectId);
  dataStore.entityNameMap.emplace(response.entityName, response.gameObjectId);
  const auto entityData = inFlightMap.at(response.requestId);
  dataStore.scene.insert({entityData.name, entityData});
  inFlightMap.erase(response.requestId);
}

void DataFacade::createStaticModel(const EntityData& entityData) noexcept {
  unsaved = true;
  engineBusy = true;
  const auto key = requestIdGenerator.getKey();
  const auto batchId = batchIdGenerator.getKey();
  inFlightMap.emplace(key, entityData);
  // TODO(resources-1): Emit a begin batch and end batch and a response handler as well
  eventQueue->emit(
      tr::StaticModelRequest{.batchId = batchId,
                             .requestId = key,
                             .modelFilename = dataStore.models.at(entityData.modelName).filePath,
                             .entityName = entityData.name,
                             .initialTransform = std::make_optional(
                                 tr::TransformData{.position = entityData.orientation.position,
                                                   .rotation = entityData.orientation.rotation})},
      "test_group");
}

void DataFacade::update() const {
}

void DataFacade::clear() {
  dataStore.animations.clear();
  dataStore.models.clear();
  dataStore.skeletons.clear();
  dataStore.scene.clear();
}

void DataFacade::createAABB() {
  // std::random_device rd;
  // std::mt19937 gen(rd());
  // std::uniform_real_distribution<float> dis(-50.f, 50.f);

  // for (int i = 0; i < 50; ++i) {
  //    const auto min = glm::vec3{dis(gen), dis(gen), dis(gen)};
  //    const auto max = min + glm::vec3{5.f, 1.f, 1.f};
  //    [[maybe_unused]] auto entityId = GameWorldFacade.createDebugAABB(min, max);
  // }
}

void DataFacade::addSkeleton(std::string_view name, const std::filesystem::path& path) {
  dataStore.skeletons.insert(
      {name.data(), SkeletonData{.name = name.data(), .filePath = path.string()}});
  unsaved = true;
}

void DataFacade::removeSkeleton([[maybe_unused]] std::string_view name) {
}

void DataFacade::addAnimation(std::string_view name, const std::filesystem::path& path) {
  dataStore.animations.insert(
      {name.data(), AnimationData{.name = name.data(), .filePath = path.string()}});
  unsaved = true;
}

void DataFacade::removeAnimation([[maybe_unused]] std::string_view name) {
}

void DataFacade::addModel(std::string_view name, const std::filesystem::path& path) {
  dataStore.models.insert({name.data(), ModelData{.name = name.data(), .filePath = path.string()}});
  unsaved = true;
}

void DataFacade::removeModel([[maybe_unused]] std::string_view name) {
}

void DataFacade::createAnimatedModel(const EntityData& entityData) {
  unsaved = true;

  const auto modelFilename = dataStore.models.at(entityData.modelName).filePath;
  const auto skeletonFilename = dataStore.skeletons.at(entityData.skeleton).filePath;
  const auto animationFilename = dataStore.animations.at(entityData.animations[0]).filePath;
  const auto entityName = entityData.name;
  // const auto requestId = requestIdGenerator.getKey();
  // eventQueue->emit(tr::DynamicModelRequest{.requestId = requestId,
  //                                          .modelFilename = modelFilename,
  //                                          .skeletonFilename = skeletonFilename,
  //                                          .animationFilename = animationFilename,
  //                                          .entityName = entityName});
}

auto DataFacade::deleteEntity(std::string_view name) noexcept -> void {
  dataStore.scene.erase(name.data());
  dataStore.entityNameMap.erase(name.data());
}

void DataFacade::addAnimationToEntity([[maybe_unused]] std::string_view entityName,
                                      [[maybe_unused]] std::string_view animationName) {
  auto& entityData = dataStore.scene.at(entityName.data());
  entityData.animations.emplace_back(animationName.data());
  unsaved = true;
}

void DataFacade::setEntitySkeleton([[maybe_unused]] std::string_view entityName,
                                   [[maybe_unused]] std::string_view skeletonName) {
  auto& entityData = dataStore.scene.at(entityName.data());
  entityData.skeleton = skeletonName.data();
  unsaved = true;
}

void DataFacade::createTerrain(std::string_view terrainName,
                               [[maybe_unused]] glm::vec3 terrainSize) {

  auto sphereInfo = tr::SdfCreateInfo{};
  sphereInfo.shapeType = tr::ShapeType::Sphere;
  sphereInfo.shapeInfo = tr::SphereInfo{.center = glm::vec3(5.f, 5.f, 5.f), .radius = 1.5f};

  auto boxInfo = tr::SdfCreateInfo{};
  boxInfo.shapeType = tr::ShapeType::Box;
  boxInfo.shapeInfo = tr::BoxInfo{.center = glm::vec3(5.f, 5.f, 5.f), .size = 1.5f};

  auto planeInfo = tr::SdfCreateInfo{};
  planeInfo.shapeType = tr::ShapeType::Plane;
  planeInfo.shapeInfo = tr::PlaneInfo{.height = 5.f, .normal = glm::vec3(0.f, 1.f, 0.f)};

  const auto requestId = requestIdGenerator.getKey();
  eventQueue->emit(tr::TerrainCreateRequest{.requestId = requestId,
                                            .name = terrainName.data(),
                                            .sdfCreateInfo = boxInfo,
                                            .chunkCount = glm::ivec3(3, 3, 3),
                                            .chunkSize = glm::ivec3(16, 16, 16)});
}

void DataFacade::save(const std::filesystem::path& outputFile) {
  auto os = std::ofstream(outputFile, std::ios::binary);
  cereal::BinaryOutputArchive output(os);
  output(dataStore);
  Log.info("Wrote binary output file to {0}", outputFile.string());
  unsaved = false;
}

void DataFacade::load(const std::filesystem::path& inputFile) {
  Log.debug("Loading Project File");
  try {
    auto is = std::ifstream(inputFile, std::ios::binary);
    cereal::BinaryInputArchive input(is);
    auto tempStore = DataStore{};
    input(tempStore);

    clear();

    for (const auto& skeleton : tempStore.skeletons | std::views::values) {
      addSkeleton(skeleton.name, skeleton.filePath);
    }

    for (const auto& animation : tempStore.animations | std::views::values) {
      addAnimation(animation.name, animation.filePath);
    }

    for (const auto& model : tempStore.models | std::views::values) {
      addModel(model.name, model.filePath);
    }

    for (const auto& entityData : tempStore.scene | std::views::values) {
      if (entityData.animations.empty()) {
        createStaticModel(entityData);
      } else {
        createAnimatedModel(entityData);
      }
    }

    unsaved = false;
  } catch (const std::exception& ex) { Log.error(ex.what()); }
}

auto DataFacade::getEntityNames() -> std::vector<std::tuple<std::string, tr::GameObjectId>> {
  auto names = std::vector<std::tuple<std::string, tr::GameObjectId>>{};
  for (const auto& [name, _] : dataStore.scene) {
    names.emplace_back(name, dataStore.entityNameMap.at(name));
  }

  for (const auto& [name, _] : dataStore.terrainMap) {
    names.emplace_back(name, dataStore.entityNameMap.at(name));
  }

  return names;
}
}
