#include "TransformSystem.hpp"
#include "components/Transform.hpp"

namespace tr {

TransformSystem::TransformSystem(std::shared_ptr<EntityService> newEntityService)
    : entityService{std::move(newEntityService)} {
}

auto TransformSystem::update() -> void {

  entityService->updateTransforms([]([[maybe_unused]] entt::entity entity, Transform& transform) {
    {
      ZoneNamedN(update, "Update Entity", true);
      // Create combined rotation matrix
      auto rotationMatrix = glm::identity<glm::mat4>();
      {
        ZoneNamedN(rotation, "Rotation", true);
        auto rotationMatrixX = glm::rotate(glm::mat4(1.0f),
                                           glm::radians(transform.rotation.x),
                                           glm::vec3{1.f, 0.f, 0.f});
        auto rotationMatrixY = glm::rotate(glm::mat4(1.0f),
                                           glm::radians(transform.rotation.y),
                                           glm::vec3{0.f, 1.f, 0.f});
        auto rotationMatrixZ = glm::rotate(glm::mat4(1.0f),
                                           glm::radians(transform.rotation.z),
                                           glm::vec3{0.f, 0.f, 1.f});
        rotationMatrix = rotationMatrixZ * rotationMatrixY * rotationMatrixX;
      }

      {
        ZoneNamedN(rest, "The Rest", true);
        // Directly set the translation matrix without extra initialization
        auto translationMatrix = glm::identity<glm::mat4>();
        translationMatrix[3] = glm::vec4(transform.position, 1.0f); // Set translation directly

        // Final transformation: translation * rotation
        transform.transformation = translationMatrix * rotationMatrix;
      }
    }
  });
}
}
