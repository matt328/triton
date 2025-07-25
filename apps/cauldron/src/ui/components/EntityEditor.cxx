#include "EntityEditor.hpp"

#include "api/fx/IEventQueue.hpp"
#include "api/gw/editordata/GameObjectData.hpp"

#include "data/TerrainData.hpp"
#include "editors/TransformInspector.hpp"
#include "ui/ImGuiStyle.hpp"
#include "ui/assets/IconsLucide.hpp"
#include "bk/Rando.hpp"

namespace ed {

EntityEditor::EntityEditor(std::shared_ptr<tr::IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
  Log.trace("Creating EntityEditor");
}

EntityEditor::~EntityEditor() {
  Log.trace("Destroying EntityEditor");
}

auto EntityEditor::bindInput() -> void {
}

auto EntityEditor::render(const tr::EditorState& editorState) -> void {

  if (ImGui::Begin(ComponentName,
                   nullptr,
                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {

    renderMenuBar();
    renderEntityList(editorState);
    ImGui::SameLine();
    renderEntityDetailView(editorState);
  }
  ImGui::End();

  renderStaticEntityDialog(editorState);

  renderAnimatedGameObjectDialog(editorState);
}

auto EntityEditor::renderStaticEntityDialog(const tr::EditorState& editorState) -> void {
  if (staticDialogInfo.shouldShow) {
    ImGui::OpenPopup("StaticGameObject");
    staticDialogInfo.shouldShow = false;
    staticDialogInfo.isOpen = true;
    staticDialogInfo.objectName = "";
    staticDialogInfo.selectedModel = {};
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal("StaticGameObject",
                             &staticDialogInfo.isOpen,
                             ImGuiConstants::ModalFlags)) {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      shouldCancel = true;
    }

    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    // Name Input
    ImGui::InputText("GameObject Name", &staticDialogInfo.objectName);

    const auto& assets = editorState.contextData.assets;
    renderAliasCombo("Model", assets.models, staticDialogInfo.selectedModel);

    ImGui::Separator();

    std::tie(shouldOk, shouldCancel) = renderOkCancelButtons();

    ImGui::EndPopup();
  }

  if (shouldOk) {
    ImGui::CloseCurrentPopup();
    staticDialogInfo.isOpen = false;
    Log.trace("Static GameObject create: objectName={}, resourceAlias={}, resourcePath={}",
              staticDialogInfo.objectName,
              staticDialogInfo.selectedModel.alias,
              staticDialogInfo.selectedModel.filePath.string());
    const auto addModel = tr::AddStaticModel{
        .name = staticDialogInfo.objectName,
        .orientation = tr::Orientation{},
        .modelName = staticDialogInfo.selectedModel.filePath.string(),
    };
    eventQueue->emit(addModel);
  }

  if (shouldCancel) {
    Log.trace("shouldCancel");
    ImGui::CloseCurrentPopup();
    staticDialogInfo.isOpen = false;
  }
}

auto EntityEditor::renderAnimatedGameObjectDialog(const tr::EditorState& editorState) -> void {
  if (animatedDialogInfo.shouldShow) {
    ImGui::OpenPopup("AnimatedGameObject");
    animatedDialogInfo.shouldShow = false;
    animatedDialogInfo.isOpen = true;
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal("AnimatedGameObject",
                             &animatedDialogInfo.isOpen,
                             ImGuiConstants::ModalFlags)) {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      shouldCancel = true;
    }

    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    ImGui::InputText("GameObject Name", &animatedDialogInfo.objectName);

    const auto& assets = editorState.contextData.assets;
    renderAliasCombo("Model", assets.models, animatedDialogInfo.selectedModel);
    renderAliasCombo("Skeleton", assets.skeletons, animatedDialogInfo.selectedSkeleton);
    renderAliasCombo("Animation", assets.animations, animatedDialogInfo.selectedAnimation);

    ImGui::Separator();

    std::tie(shouldOk, shouldCancel) = renderOkCancelButtons();

    ImGui::EndPopup();
  }

  if (shouldOk) {
    Log.trace("shouldOk");
    ImGui::CloseCurrentPopup();
    animatedDialogInfo.isOpen = false;
    Log.trace("DynamicGameObject create: objectName={}, modelAlias={}, modelPath={}, "
              "skeletonAlias={}, skeletonPath={}, animationAlias={}, animationPath={}",
              animatedDialogInfo.objectName,
              animatedDialogInfo.selectedModel.alias,
              animatedDialogInfo.selectedModel.filePath.string(),
              animatedDialogInfo.selectedSkeleton.alias,
              animatedDialogInfo.selectedSkeleton.filePath.string(),
              animatedDialogInfo.selectedAnimation.alias,
              animatedDialogInfo.selectedAnimation.filePath.string());
  }

  if (shouldCancel) {
    Log.trace("shouldCancel");
    ImGui::CloseCurrentPopup();
    animatedDialogInfo.isOpen = false;
  }
}

auto EntityEditor::renderMenuBar() -> void {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("New")) {
      if (ImGui::MenuItem("Static Model...")) {
        staticDialogInfo.shouldShow = true;
      }
      if (ImGui::MenuItem("Animated Model...")) {
        animatedDialogInfo.shouldShow = true;
      }
      if (ImGui::MenuItem("Terrain")) {
        // dataFacade->createTerrain("terrain", glm::vec3{9.f, 9.f, 9.f});
      }
      if (ImGui::MenuItem("Test Triangle")) {
        createTestTriangle();
      }
      if (ImGui::MenuItem("Debug Cube")) {
        eventQueue->emit(tr::BoxWidget{.tag = "Test Stuff",
                                       .center = glm::vec3(0.f, 0.f, 0.f),
                                       .extent = 1.f,
                                       .color = tr::Colors::Green});
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

auto EntityEditor::renderEntityList(const tr::EditorState& editorState) -> void {
  const auto entities = editorState.contextData.scene.objectNameMap;

  ImGui::BeginChild("left pane",
                    ImVec2(ImGuiConstants::SplitPaneInitialWidth, 0),
                    ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

  for (const auto& [entityName, entityId] : entities) {
    if (ImGui::Selectable(entityName.c_str(),
                          entityName == editorState.contextData.selectedEntity)) {
      Log.trace("Selected Entity: {}", entityName);
      eventQueue->emit(tr::SelectEntity{.entityId = entityName});
    }
  }

  ImGui::EndChild();
}

auto EntityEditor::renderEntityDetailView(const tr::EditorState& editorState) -> void {
  bool del = false;
  ImGui::BeginChild("item view", ImVec2(0, 0), ImGuiChildFlags_Border);
  if (editorState.contextData.selectedEntity.has_value()) {
    if (editorState.objectDataMap.contains(editorState.contextData.selectedEntity.value())) {
      auto entityData =
          editorState.objectDataMap.at(editorState.contextData.selectedEntity.value());

      // Editor Info Component
      ImGui::Text("%s", entityData.name.c_str());

      if (ImGui::Button(ICON_LC_X " Delete", ImVec2(ImGuiConstants::ButtonWidth, 0.f))) {
        del = true;
      }

      const auto transformCallback = [&, this](std::string_view name, tr::Orientation orientation) {
        const auto entityId = editorState.contextData.scene.objectNameMap.at(name.data());
        eventQueue->emit(tr::TransformObject{
            .objectId = entityId,
            .transformData = tr::TransformData{.position = orientation.position,
                                               .rotation = orientation.rotation}});
      };
      renderTransformInspector(entityData.name, &entityData.orientation, transformCallback);
    }
    const TerrainData* terrainData = nullptr;
    // dataFacade->getTerrainData(editorState.contextData.selectedEntity.value());
    if (terrainData != nullptr) {
      renderTerrainControls(terrainData);
    }
    if (del) {
      const auto entityId = editorState.contextData.scene.objectNameMap.at(
          editorState.contextData.selectedEntity.value());
      eventQueue->emit(tr::DeleteObject{.objectId = entityId});
    }
  } else {
    ImGui::Text("No Entity Selected");
  }
  ImGui::EndChild();
}

auto EntityEditor::renderTerrainControls(const TerrainData* terrainData) -> void {

  const auto terrainId = terrainData->entityId;
  static size_t selectedChunkIndex = 0;
  const auto chunkLabel = std::format("Chunk ({},{},{})",
                                      terrainData->chunkData[selectedChunkIndex].location.x,
                                      terrainData->chunkData[selectedChunkIndex].location.y,
                                      terrainData->chunkData[selectedChunkIndex].location.z);
  if (ImGui::BeginCombo("Select Chunk", chunkLabel.c_str())) {
    for (size_t i = 0; i < terrainData->chunkData.size(); ++i) {
      const auto& chunk = terrainData->chunkData[i];
      bool isSelected = (i == selectedChunkIndex);
      std::string label = "Chunk (" + std::to_string(chunk.location.x) + "," +
                          std::to_string(chunk.location.y) + "," +
                          std::to_string(chunk.location.z) + ")";
      if (ImGui::Selectable(label.c_str(), isSelected)) {
        selectedChunkIndex = i; // Update selected chunk
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  // Get the selected chunk
  const auto& selectedChunk = terrainData->chunkData[selectedChunkIndex];

  ImGui::Text("Chunk (%i, %i, %i)",
              selectedChunk.location.x,
              selectedChunk.location.y,
              selectedChunk.location.z);

  static glm::ivec3 cellPosition = {0, 0, 0};
  ImGui::DragInt3("Position##Chunk", glm::value_ptr(cellPosition), 1);

  if (ImGui::Button("Triangulate")) {
    eventQueue->emit(tr::ChunkTriangulateRequest{.terrainId = terrainId,
                                                 .chunkId = selectedChunk.entityId,
                                                 .cellPosition = cellPosition});
  }
}

auto EntityEditor::renderOkCancelButtons(float buttonWidth) -> std::pair<bool, bool> {
  bool ok = false;
  bool cancel = false;

  auto availableWidth = ImGui::GetContentRegionAvail().x;
  ImGui::SetCursorPosX(availableWidth - (buttonWidth * 2));

  if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(buttonWidth, 0.f))) {
    ok = true;
  }
  ImGui::SameLine();
  if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(buttonWidth, 0.f))) {
    cancel = true;
  }
  return {ok, cancel};
}

auto EntityEditor::createTestTriangle() -> void {
  const auto positions = std::vector<glm::vec3>{glm::vec3(-1.f, -1.f, 0.f),
                                                glm::vec3(0.f, 1.f, 0.f),
                                                glm::vec3(1.f, -1.f, 0.f)};

  const auto colors = std::vector<glm::vec4>{glm::vec4(1.f, 0.f, 0.f, 1.f),
                                             glm::vec4(0.f, 1.f, 0.f, 1.f),
                                             glm::vec4(0.f, 0.f, 1.f, 1.f)};

  const auto texCoords =
      std::vector<glm::vec2>{glm::vec2(0.f, 0.f), glm::vec2(0.5f, 1.f), glm::vec2(1.f, 0.f)};

  const auto indices = std::vector<uint32_t>{2, 1, 0};

  std::vector<std::byte> positionBytes(reinterpret_cast<const std::byte*>(positions.data()),
                                       reinterpret_cast<const std::byte*>(positions.data()) +
                                           (positions.size() * sizeof(glm::vec3)));

  std::vector<std::byte> colorBytes(reinterpret_cast<const std::byte*>(colors.data()),
                                    reinterpret_cast<const std::byte*>(colors.data()) +
                                        (colors.size() * sizeof(glm::vec4)));

  std::vector<std::byte> texCoordBytes(reinterpret_cast<const std::byte*>(texCoords.data()),
                                       reinterpret_cast<const std::byte*>(texCoords.data()) +
                                           (texCoords.size() * sizeof(glm::vec2)));

  std::vector<std::byte> indexBytes(reinterpret_cast<const std::byte*>(indices.data()),
                                    reinterpret_cast<const std::byte*>(indices.data()) +
                                        (indices.size() * sizeof(uint32_t)));

  const auto geometryData =
      tr::GeometryData{.indexData = std::make_shared<std::vector<std::byte>>(indexBytes),
                       .positionData = std::make_shared<std::vector<std::byte>>(positionBytes),
                       .colorData = std::make_shared<std::vector<std::byte>>(colorBytes),
                       .texCoordData = std::make_shared<std::vector<std::byte>>(texCoordBytes)};

  const auto entityName = std::format("Triangle-{}", bk::RandomUtils::int32InRange(1, 20));

  const auto addTriangle = tr::AddStaticGeometry{.name = entityName,
                                                 .orientation = tr::Orientation{},
                                                 .geometryData = geometryData,
                                                 .entityName = entityName};
  eventQueue->emit(addTriangle);
}

}
