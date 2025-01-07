#pragma once

#include "CommandQueue.hpp"

#include "gp/AssetManager.hpp"
#include "gp/components/EditorInfo.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/Transform.hpp"

namespace tr {

class CreateTestEntityCommand final
    : public ICommand<entt::registry&, const std::shared_ptr<AssetManager>&> {
public:
  explicit CreateTestEntityCommand(const std::string_view newName) : name{newName.data()} {
  }

  void execute(entt::registry& registry,
               [[maybe_unused]] const std::shared_ptr<AssetManager>& assetManager) const override {

    auto modelData = assetManager->createCube();

    const auto entity = registry.create();
    registry.emplace<Renderable>(entity, std::vector{modelData.meshData});
    registry.emplace<EditorInfo>(entity, name);
    registry.emplace<Transform>(entity);
  }

private:
  std::string name;
};

}
