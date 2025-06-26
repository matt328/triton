#include "EntityEditor.hpp"

#include "api/fx/IEventQueue.hpp"
#include "bk/DebugTools.hpp"

#include "data/DataStore.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/dialog/ModalDialog.hpp"
#include "editors/TransformInspector.hpp"

namespace ed {

constexpr auto DialogName = " AnimatedEntity";
constexpr auto StaticEntityDialogName = " StaticEntity";

EntityEditor::EntityEditor(std::shared_ptr<tr::IEventQueue> newEventQueue,
                           std::shared_ptr<DialogManager> newDialogManager)
    : eventQueue{std::move(newEventQueue)}, dialogManager{std::move(newDialogManager)} {
  Log.trace("Creating EntityEditor");

  createAnimatedEntityDialog();
  createStaticEntityDialog();
}

EntityEditor::~EntityEditor() {
  Log.trace("Destroying EntityEditor");
}

auto EntityEditor::render(const tr::EditorState& editorState) -> void {

  auto staticModelContext = DialogRenderContext{};
  std::vector<std::string> items = {"item1", "item2"};
  DialogRenderContext::DropdownData dropdownData = {.items = items};
  staticModelContext.dropdownMap.emplace("model", dropdownData);

  if (ImGui::Begin(ComponentName,
                   nullptr,
                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("New")) {
        if (ImGui::MenuItem("Static Model...")) {
          dialogManager->setOpen(StaticEntityDialogName, staticModelContext);
        }
        if (ImGui::MenuItem("Animated Model...")) {
          dialogManager->setOpen(DialogName, {});
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
}

void EntityEditor::createAnimatedEntityDialog() const {

  const auto onOk = [&](const ModalDialog& dialog) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-5.f, 5.f);

    for (int i = 0; i < dialog.getValue<int>("count"); ++i) {
      // emit AnimatedModelRequest
      // dataFacade->createAnimatedModel(EntityData{
      //     .name = std::format("{} {}", dialog.getValue<std::string>("name").value(), i),
      //     .orientation = Orientation{.position = glm::vec3{dis(gen), dis(gen), dis(gen)}},
      //     .modelName = dialog.getValue<std::string>("model").value(),
      //     .skeleton = dialog.getValue<std::string>("skeleton").value(),
      //     .animations = {dialog.getValue<std::string>("animation").value()}});
    }
  };

  const auto onCancel = []() { Log.debug("Cancelled Dialog with no input"); };

  auto dialog = std::make_unique<ModalDialog>(ICON_LC_PLAY, DialogName, onOk, onCancel);

  dialog->addControl("name", "Entity Name", std::string{"Unnamed Entity"});

  dialog->addControl("model", "Model Name", std::string{"Unnamed Model"});

  dialog->addControl("skeleton", "Skeleton Name", std::string{"Unnamed Skeleton"});

  dialog->addControl("animation", "Animation Name", std::string{"Unnamed Animation"});

  const int defaultCount = 10;
  dialog->addControl("count", "Count", defaultCount);

  dialogManager->addDialog(DialogName, std::move(dialog));
}

void EntityEditor::createStaticEntityDialog() const {

  const auto onOk = [&](const ModalDialog& dialog) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-5.f, 5.f);
    for (int i = 0; i < dialog.getValue<int>("count"); ++i) {
      // emit StaticModelRequest
      // dataFacade->createStaticModel(EntityData{
      //     .name = std::format("{} {}", dialog.getValue<std::string>("name").value(), i),
      //     .orientation = Orientation{.position = glm::vec3{dis(gen), dis(gen), dis(gen)}},
      //     .modelName = dialog.getValue<std::string>("model").value(),
      //     .skeleton = "",
      //     .animations = {}});
    }
  };

  const auto onCancel = []() { Log.debug("Cancelled Dialog with no input"); };

  auto dialog =
      std::make_unique<ModalDialog>(ICON_LC_CUBOID, StaticEntityDialogName, onOk, onCancel);
  dialog->addControl("name", "Entity Name", std::string{"Unnamed Entity"});
  dialog->addControl("model", "Model Name", std::string{"Unnamed Model"});
  const int defaultCount = 10;
  dialog->addControl("count", "Count", defaultCount);

  dialogManager->addDialog(StaticEntityDialogName, std::move(dialog));
}

}
