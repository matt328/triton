#pragma once

#include "entt/fwd.hpp"
#include "ctx/GameplayFacade.hpp"

namespace ed::io {

   static constexpr auto EditorInfo = "editor-info";

   void writeProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade);
   void readProjectFile(std::string_view filename, tr::ctx::GameplayFacade& facade);

   void parseEditorInfoComponent(const nlohmann::basic_json<nlohmann::ordered_map>& componentJson,
                                 std::optional<tr::ctx::EditorInfoComponent>& editorInfo);

   void parseCameraComponent(const nlohmann::basic_json<nlohmann::ordered_map>& componentJson,
                             std::optional<tr::gp::ecs::Camera>& cameraInfo);

   void parseTransformComponent(const nlohmann::basic_json<nlohmann::ordered_map>& componentJson,
                                std::optional<tr::gp::ecs::Transform>& transformInfo);
}