#pragma once

#include "CommandQueue.hpp"
#include "gp/AssetManager.hpp"
#include "gp/components/EditorInfo.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/Transform.hpp"

namespace tr {

class CreateStaticEntityCommand final
    : public ICommand<entt::registry&, const std::shared_ptr<AssetManager>&> {
public:
  explicit CreateStaticEntityCommand(const std::string_view newModelFilename,
                                     const std::string_view newEntityName)
      : entityName{newEntityName.data()}, modelFilename{newModelFilename.data()} {
  }

  void execute([[maybe_unused]] entt::registry& registry,
               [[maybe_unused]] const std::shared_ptr<AssetManager>& assetManager) const override {

    auto modelData = assetManager->loadModel(modelFilename);

    auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                               .position = {0.f, 0.f, -35.f},
                               .transformation = glm::identity<glm::mat4>()};

    const auto entity = registry.create();
    registry.emplace<Renderable>(entity, std::vector{modelData.meshData});
    registry.emplace<Transform>(entity, transform);
    registry.emplace<EditorInfo>(entity, entityName);
  }

private:
  std::string entityName;
  std::string modelFilename;
};

}
