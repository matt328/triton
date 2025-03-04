#include "EntityEditor.hpp"

#include "tr/GameplayEvents.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/dialog/ModalDialog.hpp"
#include "editors/TransformInspector.hpp"

namespace ed {

constexpr auto DialogName = " AnimatedEntity";
constexpr auto StaticEntityDialogName = " StaticEntity";

EntityEditor::EntityEditor(std::shared_ptr<tr::IGameplaySystem> newGameplaySystem,
                           std::shared_ptr<DataFacade> newDataFacade,
                           std::shared_ptr<DialogManager> newDialogManager,
                           std::shared_ptr<tr::IEventBus> newEventBus,
                           std::shared_ptr<tr::EntityService> newEntityService)
    : gameplaySystem{std::move(newGameplaySystem)},
      dataFacade{std::move(newDataFacade)},
      dialogManager{std::move(newDialogManager)},
      eventBus{std::move(newEventBus)},
      entityService{std::move(newEntityService)} {
  Log.trace("Creating EntityEditor");

  eventBus->subscribe<tr::EntityCreated>([](const tr::EntityCreated& event) {
    Log.trace("EntityEditor entityCreated: {}", static_cast<long>(event.entityId));
  });

  createAnimatedEntityDialog();
  createStaticEntityDialog();
}

EntityEditor::~EntityEditor() {
  Log.trace("Destroying EntityEditor");
}

void EntityEditor::render() {

  if (ImGui::Begin(ComponentName,
                   nullptr,
                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("New")) {
        if (ImGui::MenuItem("Static Model...")) {
          dialogManager->setOpen(StaticEntityDialogName);
        }
        if (ImGui::MenuItem("Animated Model...")) {
          dialogManager->setOpen(DialogName);
        }
        if (ImGui::MenuItem("Terrain")) {
          dataFacade->createTerrain("terrain", glm::vec3{9.f, 9.f, 9.f});
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    const auto entities = dataFacade->getEntityNames();

    // Left
    ImGui::BeginChild("left pane",
                      ImVec2(150, 0),
                      ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

    for (const auto& [entityName, entityId] : entities) {
      if (ImGui::Selectable(entityName.c_str(), entityName == selectedEntity)) {
        Log.trace("Selected Entity: {}", entityName);
        selectedEntity = entityName;
      }
    }

    ImGui::EndChild();
    ImGui::SameLine();

    bool del = false;
    // Right
    {
      // Header
      ImGui::BeginChild("item view", ImVec2(0, 0), ImGuiChildFlags_Border);
      if (selectedEntity.has_value()) {

        auto* entityData = dataFacade->getEntityData(selectedEntity.value());

        if (entityData != nullptr) {
          // Editor Info Component
          ImGui::Text("%s", entityData->name.c_str());

          auto buttonWidth = 120.f;
          if (ImGui::Button(ICON_LC_X " Delete", ImVec2(buttonWidth, 0.f))) {
            del = true;
          }

          const auto transformCallback = [this](std::string_view name, Orientation orientation) {
            const auto entityId = dataFacade->getEntityId(name);
            entityService->setTransform(
                entityId,
                tr::Transform{.rotation = orientation.rotation, .position = orientation.position});
          };
          renderTransformInspector(entityData->name, &entityData->orientation, transformCallback);
        }

        // Terrain Controls
        const auto* terrainData = dataFacade->getTerrainData(selectedEntity.value());
        if (terrainData != nullptr) {
          for (const auto id : terrainData->chunkIds) {
            // TODO(matt): Change this to a select when there are more chunks
            ImGui::Text("Chunk ID: %i", id);

            /* TODO(matt): Tomorrow:

              First we need some parameters to generate an SDF.
              Probably need an SDF Manager, and a struct of parameters that can evolve into a
              piecewise function definition that the SDF Manager can use to generate the SDF.
              It probably won't generate the entire SDF all at once, but rather expose and API to
              sample a continuous function to produce a coeherent SDF. For now hard code the terrain
              manager to pass an empty struct to the SDFManager to configure it, then the SDF
              Manager should provide an API to quickly sample the field at various coordinates.
              Hardcode the sdf manager to produce a simple horizontal plane at a a Y value given
              supplied in the empty struct mentioned earlier.
              - Based on the current chunk's location, allow the UI to step through one cell at a
              time and generate the vertices needed for the cell.
              - Render debug indicators such as the bounds of the current cell itself, vertices as
              points and the actual surface itself.
              - Allow the UI to see the cells that are not completely air or matter in a list, and
              selecting a cell highlights these debug indicators on the 3d view.
              - This should be a useful tool throughout most of the terrain implementation process
              - The UI should also include debug info about the current cell and possibly each cell
              corner such as the equivalance class and all that.

             */
          }
        }

        if (del) {
          const auto entityId = dataFacade->getEntityId(selectedEntity.value());
          dataFacade->deleteEntity(selectedEntity.value());
          entityService->removeEntity(entityId);
          selectedEntity = std::nullopt;
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
  ValueProvider modelProvider = [this]() -> std::vector<std::string> {
    const auto& models = dataFacade->getModels();
    auto modelNames = std::vector<std::string>{};
    modelNames.reserve(models.size());
    std::ranges::transform(models, std::back_inserter(modelNames), [](const auto& pair) {
      return pair.first;
    });
    return modelNames;
  };

  ValueProvider skeletonProvider = [this]() -> std::vector<std::string> {
    const auto& skeletons = dataFacade->getSkeletons();
    auto skeletonNames = std::vector<std::string>{};
    skeletonNames.reserve(skeletons.size());
    std::ranges::transform(skeletons, std::back_inserter(skeletonNames), [](const auto& pair) {
      return pair.first;
    });
    return skeletonNames;
  };

  ValueProvider animationsProvider = [this]() -> std::vector<std::string> {
    const auto& animations = dataFacade->getAnimations();
    auto animationNames = std::vector<std::string>{};
    animationNames.reserve(animations.size());
    std::ranges::transform(animations, std::back_inserter(animationNames), [](const auto& pair) {
      return pair.first;
    });
    return animationNames;
  };

  const auto onOk = [&](const ModalDialog& dialog) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-5.f, 5.f);

    for (int i = 0; i < dialog.getValue<int>("count"); ++i) {
      dataFacade->createAnimatedModel(EntityData{
          .name = fmt::format("{} {}", dialog.getValue<std::string>("name").value(), i),
          .orientation = Orientation{.position = glm::vec3{dis(gen), dis(gen), dis(gen)}},
          .modelName = dialog.getValue<std::string>("model").value(),
          .skeleton = dialog.getValue<std::string>("skeleton").value(),
          .animations = {dialog.getValue<std::string>("animation").value()}});
    }
  };

  const auto onCancel = []() { Log.debug("Cancelled Dialog with no input"); };

  auto dialog = std::make_unique<ModalDialog>(ICON_LC_PLAY, DialogName, onOk, onCancel);

  dialog->addControl("name", "Entity Name", std::string{"Unnamed Entity"});

  dialog->addControl("model", "Model Name", std::string{"Unnamed Model"}, modelProvider);

  dialog->addControl("skeleton",
                     "Skeleton Name",
                     std::string{"Unnamed Skeleton"},
                     skeletonProvider);

  dialog->addControl("animation",
                     "Animation Name",
                     std::string{"Unnamed Animation"},
                     animationsProvider);

  const int defaultCount = 10;
  dialog->addControl("count", "Count", defaultCount);

  dialogManager->addDialog(DialogName, std::move(dialog));
}

void EntityEditor::createStaticEntityDialog() const {
  ValueProvider modelProvider = [this]() -> std::vector<std::string> {
    const auto& models = dataFacade->getModels();
    auto modelNames = std::vector<std::string>{};
    modelNames.reserve(models.size());
    std::ranges::transform(models, std::back_inserter(modelNames), [](const auto& pair) {
      return pair.first;
    });
    return modelNames;
  };

  const auto onOk = [&](const ModalDialog& dialog) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-5.f, 5.f);
    for (int i = 0; i < dialog.getValue<int>("count"); ++i) {
      dataFacade->createStaticModel(EntityData{
          .name = fmt::format("{} {}", dialog.getValue<std::string>("name").value(), i),
          .orientation = Orientation{.position = glm::vec3{dis(gen), dis(gen), dis(gen)}},
          .modelName = dialog.getValue<std::string>("model").value(),
          .skeleton = "",
          .animations = {}});
    }
  };

  const auto onCancel = []() { Log.debug("Cancelled Dialog with no input"); };

  auto dialog =
      std::make_unique<ModalDialog>(ICON_LC_CUBOID, StaticEntityDialogName, onOk, onCancel);
  dialog->addControl("name", "Entity Name", std::string{"Unnamed Entity"});
  dialog->addControl("model", "Model Name", std::string{"Unnamed Model"}, modelProvider);
  const int defaultCount = 10;
  dialog->addControl("count", "Count", defaultCount);

  dialogManager->addDialog(StaticEntityDialogName, std::move(dialog));
}

}
