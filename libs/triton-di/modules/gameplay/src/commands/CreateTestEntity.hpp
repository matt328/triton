#pragma once

#include "../CommandQueue.hpp"
#include <entt/entity/fwd.hpp>

#include "gp/components/EditorInfo.hpp"
#include "gp/components/Transform.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {
class CreateTestEntityCommand final
    : public ICommand<entt::registry&, const std::shared_ptr<VkResourceManager>&> {
public:
  explicit CreateTestEntityCommand(const std::string_view newName) : name{newName.data()} {
  }

  void execute(
      entt::registry& registry,
      [[maybe_unused]] const std::shared_ptr<VkResourceManager>& resourceManager) const override {
    const auto entity = registry.create();
    registry.emplace<EditorInfo>(entity, name);
    registry.emplace<Transform>(entity);
  }

private:
  std::string name;
};
}
