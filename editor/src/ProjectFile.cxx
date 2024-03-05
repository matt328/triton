#include "ProjectFile.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include "ctx/GameplayFacade.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Transform.hpp"

namespace ed::io {
   using nlohmann::ordered_json;
   void writeProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade) {
      ordered_json j;

      j["name"] = "project name";
      j["activeCamera"] = "Camera #1";

      const auto& entities = facade.getAllEntities();

      for (const auto& entity : entities) {
         ordered_json je{};
         const auto maybeTransform = facade.getComponent<tr::gp::ecs::Transform>(entity);
         if (maybeTransform.has_value()) {
            const auto transform = maybeTransform.value().get();
            je["components"] = {{{"type", "transform"},
                                 {"position",
                                  {{"x", transform.position.x},
                                   {"y", transform.position.y},
                                   {"z", transform.position.z}}},
                                 {"rotation",
                                  {{"x", transform.rotation.x},
                                   {"y", transform.rotation.y},
                                   {"z", transform.rotation.z}}}}};
         }

         const auto maybeEntityInfo = facade.getComponent<tr::ctx::EditorInfoComponent>(entity);
         if (maybeEntityInfo.has_value()) {
            const auto entityInfo = maybeEntityInfo.value().get();
            ordered_json editorInfo{};
            editorInfo["type"] = "editor-info";
            editorInfo["name"] = entityInfo.name;

            if (entityInfo.sourceMesh.has_value()) {
               editorInfo["sourceMesh"] = entityInfo.sourceMesh.value();
            }

            if (entityInfo.sourceTexture.has_value()) {
               editorInfo["sourceTexture"] = entityInfo.sourceTexture.value();
            }

            je["components"].push_back(editorInfo);
         }

         const auto maybeCameraComponent = facade.getComponent<tr::gp::ecs::Camera>(entity);
         if (maybeCameraComponent.has_value()) {
            const auto cameraComponent = maybeCameraComponent.value().get();
            ordered_json cameraInfo{};
            cameraInfo["type"] = "camera";

            cameraInfo["yaw"] = cameraComponent.yaw;
            cameraInfo["pitch"] = cameraComponent.pitch;
            cameraInfo["fov"] = cameraComponent.fov;

            cameraInfo["position"] = {{{"x", cameraComponent.position.x},
                                       {"y", cameraComponent.position.y},
                                       {"z", cameraComponent.position.z}}};

            je["components"].push_back(cameraInfo);
         }

         j["entities"].push_back(je);
      }
      Log::info << j.dump(2) << std::endl;
   }

   void readProjectFile(entt::registry& registry, std::string_view& filename) {
   }
}