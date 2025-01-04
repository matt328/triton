#pragma once

#include "CommandQueue.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class CreateStaticEntityCommand final
    : public ICommand<entt::registry&, const std::shared_ptr<VkResourceManager>&> {
public:
  explicit CreateStaticEntityCommand(const std::string_view newModelFilename,
                                     const std::string_view newEntityName)
      : entityName{newEntityName.data()}, modelFilename{newModelFilename.data()} {
  }

  void execute(
      [[maybe_unused]] entt::registry&,
      [[maybe_unused]] const std::shared_ptr<VkResourceManager>& resourceManager) const override {

    // const auto modelData = resourceManager->createModel(std::filesystem::path{modelFilename});
    // const auto entity = registry.create();
    // registry.emplace<Renderable>(entity, std::vector{modelData.meshData});
    // registry.emplace<Transform>(entity);
    // registry.emplace<EditorInfo>(entity, entityName);
  }

private:
  std::string entityName;
  std::string modelFilename;
};

}
