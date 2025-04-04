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

        auto& entityData = dataFacade->getEntityData(selectedEntity.value());

        // Editor Info Component
        ImGui::Text("%s", entityData.name.c_str());

        auto buttonWidth = 120.f;
        if (ImGui::Button(ICON_LC_X " Delete", ImVec2(buttonWidth, 0.f))) {
          del = true;
        }

        // Transform Component

        const auto transformCallback = [this](std::string_view name, Orientation orientation) {
          const auto entityId = dataFacade->getEntityId(name);
          entityService->setTransform(
              entityId,
              tr::Transform{.rotation = orientation.rotation, .position = orientation.position});
        };

        renderTransformInspector(entityData.name, &entityData.orientation, transformCallback);

        /*
          None of the other components yet have much functionality that makes sense to be changed
          from the editor. As more of the gameplay module is implemented, more requirements for
          these component editors will be realized.
        */

        // // Camera Component
        // if (auto* camera = registry->try_get<tr::Camera>(selectedEntity.value());
        //     camera != nullptr) {
        //   renderCameraInspector(camera);
        // }

        // // Animation Component
        // if (auto* animation = registry->try_get<tr::Animation>(selectedEntity.value());
        //     animation != nullptr) {
        //   renderAnimationInspector(animation);
        // }

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
