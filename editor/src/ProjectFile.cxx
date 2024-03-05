#include "ProjectFile.hpp"

#include "ctx/GameplayFacade.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Transform.hpp"
#include <nlohmann/json_fwd.hpp>

namespace ed::io {
   using nlohmann::ordered_json;
   void writeProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade) {
      ordered_json rootJson;

      rootJson["name"] = "project name";
      rootJson["activeCamera"] = facade.getActiveCameraName();

      const auto& entities = facade.getAllEntities();

      for (const auto& entity : entities) {
         ordered_json entityJson{};
         const auto maybeTransform = facade.getComponent<tr::gp::ecs::Transform>(entity);
         if (maybeTransform.has_value()) {
            const auto& transform = maybeTransform.value().get();
            entityJson["components"] = {{{"type", "transform"},
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
            ordered_json editorInfoJson{};
            editorInfoJson["type"] = "editor-info";
            editorInfoJson["name"] = entityInfo.name;

            if (entityInfo.sourceMesh.has_value()) {
               editorInfoJson["sourceMesh"] = entityInfo.sourceMesh.value();
            }

            if (entityInfo.sourceTexture.has_value()) {
               editorInfoJson["sourceTexture"] = entityInfo.sourceTexture.value();
            }

            entityJson["components"].push_back(editorInfoJson);
         }

         const auto maybeCameraComponent = facade.getComponent<tr::gp::ecs::Camera>(entity);
         if (maybeCameraComponent.has_value()) {
            const auto cameraComponent = maybeCameraComponent.value().get();
            ordered_json cameraInfoJson{};
            cameraInfoJson["type"] = "camera";

            cameraInfoJson["yaw"] = cameraComponent.yaw;
            cameraInfoJson["pitch"] = cameraComponent.pitch;
            cameraInfoJson["fov"] = cameraComponent.fov;

            cameraInfoJson["position"] = {{"x", cameraComponent.position.x},
                                          {"y", cameraComponent.position.y},
                                          {"z", cameraComponent.position.z}};

            entityJson["components"].push_back(cameraInfoJson);
         }

         rootJson["entities"].push_back(entityJson);

         std::ofstream o(filename.data());

         o << std::setw(2) << rootJson << std::endl;

         o.close();
      }
      Log::info << rootJson.dump(2) << std::endl;
   }

   void readProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade) {
      std::ifstream i(filename.data());
      ordered_json rootJson{};
      i >> rootJson;

      for (const auto& entityJson : rootJson["entities"]) {
         auto entityName = std::optional<std::string>{};

         for (const auto& componentJson : entityJson["components"]) {
            auto sourceMesh = std::optional<std::string>{};
            auto sourceTexture = std::optional<std::string>{};

            const auto cType = componentJson["type"];
            const auto typeStr = cType.template get<std::string>();

            if (typeStr == "editor-info") {
               entityName = componentJson["name"].template get<std::string>();
               sourceMesh = componentJson.contains("sourceMesh")
                                ? componentJson["sourceMesh"].template get<std::string>()
                                : std::optional<std::string>{};
               sourceTexture = componentJson.contains("sourceTexture")
                                   ? componentJson["sourceTexture"].template get<std::string>()
                                   : std::optional<std::string>{};
            }

            if (typeStr == "camera") {
               float fov = componentJson["fov"];
               float yaw = componentJson["yaw"];
               float pitch = componentJson["pitch"];
               auto posJson = componentJson["position"];

               float x = posJson["x"];
               float y = posJson["y"];
               float z = posJson["z"];

               auto position = glm::vec3{x, y, z};
               // TODO: createCamera overload that takes yaw and pitch, and doesn't need width,
               // height, near or far
               uint32_t width = 1920;
               uint32_t height = 1080;
               auto cam =
                   facade.createCamera(width, height, fov, 0.1f, 100.f, position, entityName);
               if (rootJson["activeCamera"] == entityName.value()) {
                  facade.setCurrentCamera(cam);
               }
            }

            if (typeStr == "transform") {
               auto posJson = componentJson["position"];
               float x = posJson["x"];
               float y = posJson["y"];
               float z = posJson["z"];
               auto position = glm::vec3{x, y, z};

               auto rotJson = componentJson["rotation"];
               float xr = rotJson["x"];
               float yr = rotJson["y"];
               float zr = rotJson["z"];
               auto rotation = glm::vec3{xr, yr, zr};

               auto e = facade.createStaticMeshEntity(sourceMesh.value(),
                                                      sourceTexture.value(),
                                                      entityName);
               auto& newTransform = facade.getComponent<tr::gp::ecs::Transform>(e).value().get();
               newTransform.position = position;
               newTransform.position = rotation;
            }
         }
      }

      i.close();
   }
}