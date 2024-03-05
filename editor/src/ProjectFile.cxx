#include "ProjectFile.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace ed::io {
   using nlohmann::json;
   void writeProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade) {
      json j;

      j["name"] = "project name";
      j["activeCamera"] = "Camera #1";

      const auto& entities = facade.getAllEntities();

      for (const auto& entity : entities) {
         json je{};
         const auto transform = facade.getEntityTransform(entity);
         const auto entityInfo = facade.getEditorInfo(entity);
         je["components"] = {{{"type", "transform"},
                              {"position",
                               {{"x", transform.position.x},
                                {"y", transform.position.y},
                                {"z", transform.position.z}}},
                              {"rotation",
                               {{"x", transform.rotation.x},
                                {"y", transform.rotation.y},
                                {"z", transform.rotation.z}}}}};

         json editorInfo{};
         editorInfo["type"] = "editor-info";
         editorInfo["name"] = entityInfo.name;

         if (entityInfo.sourceMesh.has_value()) {
            editorInfo["sourceMesh"] = entityInfo.sourceMesh.value();
         }

         if (entityInfo.sourceTexture.has_value()) {
            editorInfo["sourceTexture"] = entityInfo.sourceTexture.value();
         }

         je["components"].push_back(editorInfo);

         const auto cameraComponent = facade.getCameraComponent(entity);
         if (cameraComponent.has_value()) {
            json cameraInfo{};
            cameraInfo["type"] = "camera";
            cameraInfo["yaw"] = cameraComponent.value().get().yaw;
            cameraInfo["pitch"] = cameraComponent.value().get().pitch;
            cameraInfo["fov"] = cameraComponent.value().get().fov;
         }

         j["entities"].push_back(je);
      }
      Log::info << j.dump(2) << std::endl;
   }

   void readProjectFile(entt::registry& registry, std::string_view& filename) {
   }
}