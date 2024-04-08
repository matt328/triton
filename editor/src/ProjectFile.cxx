#include "ProjectFile.hpp"

#include "ctx/GameplayFacade.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Transform.hpp"
#include <nlohmann/json_fwd.hpp>
#include "gfx/Handles.hpp"

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
            editorInfoJson["type"] = EditorInfo;
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
   }

   void readProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade) {
      std::ifstream i(filename.data());
      ordered_json rootJson{};
      i >> rootJson;

      for (const auto& entityJson : rootJson["entities"]) {
         auto editorInfoComponent = std::optional<tr::ctx::EditorInfoComponent>{};
         auto transformComponent = std::optional<tr::gp::ecs::Transform>{};
         auto cameraComponent = std::optional<tr::gp::ecs::Camera>{};

         for (const auto& componentJson : entityJson["components"]) {
            const std::string typeStr = componentJson["type"];

            if (typeStr == EditorInfo) {
               parseEditorInfoComponent(componentJson, editorInfoComponent);
            }

            if (typeStr == "camera") {
               parseCameraComponent(componentJson, cameraComponent);
            }

            if (typeStr == "transform") {
               parseTransformComponent(componentJson, transformComponent);
            }
         }

         // Camera Entity
         if (cameraComponent.has_value()) {
            const auto c = cameraComponent.value();
            const auto name = editorInfoComponent.has_value()
                                  ? std::optional<std::string>{editorInfoComponent.value().name}
                                  : std::nullopt;
            auto cam = facade.createCamera(c.width,
                                           c.height,
                                           c.fov,
                                           c.nearClip,
                                           c.farClip,
                                           c.position,
                                           name);

            std::string activeCameraStr = rootJson["activeCamera"];

            if (activeCameraStr == name.value()) {
               facade.setCurrentCamera(cam);
            }
         }

         // Static Mesh Entity
         if (editorInfoComponent.has_value() &&
             editorInfoComponent.value().sourceMesh.has_value() &&
             editorInfoComponent.value().sourceTexture.has_value()) {

            //

            // const auto meshHandle =
            //     static_cast<tr::gfx::MeshHandle>(editorInfoComponent.value().sourceMesh.value());

            // auto map = std::unordered_map<tr::gfx::MeshHandle, tr::gfx::TextureHandle>{
            //     {editorInfoComponent.value().sourceMesh.value(),
            //      editorInfoComponent.value().sourceTexture.value()}};
            // auto e = facade.createStaticMeshEntity(map);
            // if (transformComponent.has_value()) {
            //    auto& newTransform = facade.getComponent<tr::gp::ecs::Transform>(e).value().get();
            //    newTransform.position = transformComponent.value().position;
            //    newTransform.position = transformComponent.value().rotation;
            // }
         }
      }

      i.close();
   }

   void parseEditorInfoComponent(const nlohmann::basic_json<nlohmann::ordered_map>& componentJson,
                                 std::optional<tr::ctx::EditorInfoComponent>& editorInfo) {
      std::string name = componentJson["name"];
      auto sourceMesh = componentJson.contains("sourceMesh")
                            ? std::optional<std::string>{componentJson["sourceMesh"]}
                            : std::nullopt;
      auto sourceTexture = componentJson.contains("sourceTexture")
                               ? std::optional<std::string>{componentJson["sourceTexture"]}
                               : std::nullopt;
      editorInfo.emplace(name, sourceMesh, sourceTexture);
   }

   void parseCameraComponent(const nlohmann::basic_json<nlohmann::ordered_map>& componentJson,
                             std::optional<tr::gp::ecs::Camera>& cameraInfo) {
      float fov = componentJson["fov"];
      [[maybe_unused]] float yaw = componentJson["yaw"];
      [[maybe_unused]] float pitch = componentJson["pitch"];
      auto posJson = componentJson["position"];

      float x = posJson["x"];
      float y = posJson["y"];
      float z = posJson["z"];

      auto position = glm::vec3{x, y, z};
      // TODO: createCamera overload that takes yaw and pitch, and doesn't need width,
      // height, near or far, can get these values from the context
      uint32_t width = 1920;
      uint32_t height = 1080;
      float nearClip = 0.1f;
      float farClip = 100000.f;

      cameraInfo.emplace(width, height, fov, nearClip, farClip, position);
   }

   void parseTransformComponent(const nlohmann::basic_json<nlohmann::ordered_map>& componentJson,
                                std::optional<tr::gp::ecs::Transform>& transformInfo) {
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

      transformInfo.emplace(tr::gp::ecs::Transform{position, rotation});
   }
}