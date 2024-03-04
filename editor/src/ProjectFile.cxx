#include "ProjectFile.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace ed::io {
   using nlohmann::json;
   void writeProjectFile(std::string_view filename) {
      json j;

      j["name"] = "project name";
      j["activeCamera"] = "Camera #1";

      json entity1, entity2;
      entity1["components"] = {{{"type", "transform"},
                                {"position", {{"x", 0.0}, {"y", 0.0}, {"z", 0.0}}},
                                {"rotation", {{"x", 0.0}, {"y", 0.0}, {"z", 0.0}}}},
                               {{"type", "editor-info"}, {"name", "some-entity"}},
                               {{"type", "renderable-info"},
                                {"sourceMesh", "relative filename"},
                                {"sourceTexture", "relative filename"}}};

      entity2["components"] = {{{"type", "editor-info"}, {"name", "entity name"}},
                               {{"type", "transform"},
                                {"position", {{"x", 0.0}, {"y", 0.0}, {"z", 0.0}}},
                                {"rotation", {{"x", 0.0}, {"y", 0.0}, {"z", 0.0}}}},
                               {{"type", "camera"}, {"yaw", -90.0}, {"pitch", 0.0}, {"fov", 60.0}}};

      j["entities"] = {entity1, entity2};
      Log::info << j.dump(4) << std::endl;
   }

   void readProjectFile(entt::registry& registry, std::string_view& filename) {
   }
}