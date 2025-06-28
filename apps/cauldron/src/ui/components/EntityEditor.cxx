#include "EntityEditor.hpp"

#include "api/fx/IEventQueue.hpp"
#include "api/gw/editordata/GameObjectData.hpp"

#include "data/DataStore.hpp"
#include "editors/TransformInspector.hpp"
#include "ui/assets/IconsLucide.hpp"

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

    const auto entities = editorState.contextData.scene.objectNameMap;

    // Left
    ImGui::BeginChild("left pane",
                      ImVec2(150, 0),
                      ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

    for (const auto& [entityName, entityId] : entities) {
      if (ImGui::Selectable(entityName.c_str(),
                            entityName == editorState.contextData.selectedEntity)) {
        Log.trace("Selected Entity: {}", entityName);
        eventQueue->emit(tr::SelectEntity{.entityId = entityName});
      }
    }

    ImGui::EndChild();
    ImGui::SameLine();

    bool del = false;
    // Right
    {
      // Header
      ImGui::BeginChild("item view", ImVec2(0, 0), ImGuiChildFlags_Border);
      if (editorState.contextData.selectedEntity.has_value()) {
        if (editorState.objectDataMap.contains(editorState.contextData.selectedEntity.value())) {
          auto entityData =
              editorState.objectDataMap.at(editorState.contextData.selectedEntity.value());

          // Editor Info Component
          ImGui::Text("%s", entityData.name.c_str());

          auto buttonWidth = 120.f;
          if (ImGui::Button(ICON_LC_X " Delete", ImVec2(buttonWidth, 0.f))) {
            del = true;
          }

          const auto transformCallback = [&, this](std::string_view name,
                                                   tr::Orientation orientation) {
            const auto entityId = editorState.contextData.scene.objectNameMap.at(name.data());
            eventQueue->emit(tr::TransformObject{
                .objectId = entityId,
                .transformData = tr::TransformData{.position = orientation.position,
                                                   .rotation = orientation.rotation}});
          };
          renderTransformInspector(entityData.name, &entityData.orientation, transformCallback);
        }
        // Terrain Controls
        const TerrainData* terrainData = nullptr;
        // dataFacade->getTerrainData(editorState.contextData.selectedEntity.value());
        if (terrainData != nullptr) {
          const auto terrainId = terrainData->entityId;
          static size_t selectedChunkIndex = 0;
          const auto chunkLabel =
              std::format("Chunk ({},{},{})",
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

        if (del) {
          const auto entityId = editorState.contextData.scene.objectNameMap.at(
              editorState.contextData.selectedEntity.value());
          eventQueue->emit(tr::DeleteObject{.objectId = entityId});
        }
      } else {
        ImGui::Text("No Entity Selected");
      }
    }
    ImGui::EndChild();
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
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal("StaticGameObject", &staticDialogInfo.isOpen)) {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      shouldCancel = true;
    }

    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    // Name Input
    ImGui::InputText("GameObject Name", &staticDialogInfo.objectName);

    // Model Name/Path Input
    if (ImGui::BeginCombo("Model", staticDialogInfo.selectedModel.alias.c_str())) {
      const auto& options = editorState.contextData.assets.models;
      for (const auto& option : options) {
        bool isSelected = (staticDialogInfo.selectedModel == option.second);
        if (ImGui::Selectable(option.second.alias.c_str(), isSelected)) {
          staticDialogInfo.selectedModel = option.second;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    ImGui::Separator();

    auto availableWidth = ImGui::GetContentRegionAvail().x;
    auto buttonWidth = 80.f;

    ImGui::SetCursorPosX(availableWidth - (buttonWidth * 2));

    if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(buttonWidth, 0.f))) {
      shouldOk = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(buttonWidth, 0.f))) {
      shouldCancel = true;
    }

    ImGui::EndPopup();
  }

  if (shouldOk) {
    Log.trace("shouldOk");
    ImGui::CloseCurrentPopup();
    staticDialogInfo.isOpen = false;
    Log.trace("Static GameObject create: objectName={}, resourceAlias={}, resourcePath={}",
              staticDialogInfo.objectName,
              staticDialogInfo.selectedModel.alias,
              staticDialogInfo.selectedModel.filePath.string());
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

  if (ImGui::BeginPopupModal("AnimatedGameObject", &animatedDialogInfo.isOpen)) {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      shouldCancel = true;
    }

    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    // Name Input
    ImGui::InputText("GameObject Name", &animatedDialogInfo.objectName);

    // Model Name/Path Input
    if (ImGui::BeginCombo("Model", animatedDialogInfo.selectedModel.alias.c_str())) {
      const auto& options = editorState.contextData.assets.models;
      for (const auto& option : options) {
        bool isSelected = (animatedDialogInfo.selectedModel == option.second);
        if (ImGui::Selectable(option.second.alias.c_str(), isSelected)) {
          animatedDialogInfo.selectedModel = option.second;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    // Skeleton Name/Path Input
    if (ImGui::BeginCombo("Skeleton", animatedDialogInfo.selectedSkeleton.alias.c_str())) {
      const auto& options = editorState.contextData.assets.skeletons;
      for (const auto& option : options) {
        bool isSelected = (animatedDialogInfo.selectedSkeleton == option.second);
        if (ImGui::Selectable(option.second.alias.c_str(), isSelected)) {
          animatedDialogInfo.selectedSkeleton = option.second;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    // Animation Name/Path Input
    if (ImGui::BeginCombo("Animation", animatedDialogInfo.selectedAnimation.alias.c_str())) {
      const auto& options = editorState.contextData.assets.animations;
      for (const auto& option : options) {
        bool isSelected = (animatedDialogInfo.selectedAnimation == option.second);
        if (ImGui::Selectable(option.second.alias.c_str(), isSelected)) {
          animatedDialogInfo.selectedAnimation = option.second;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    ImGui::Separator();

    auto availableWidth = ImGui::GetContentRegionAvail().x;
    auto buttonWidth = 80.f;

    ImGui::SetCursorPosX(availableWidth - (buttonWidth * 2));

    if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(buttonWidth, 0.f))) {
      shouldOk = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(buttonWidth, 0.f))) {
      shouldCancel = true;
    }

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

}
