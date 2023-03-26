#pragma once

struct Renderable {
   explicit Renderable(const std::string_view& meshId, const std::string_view& textureId) :
       meshId(meshId.data()), textureId(textureId.data()) {
   }

   std::string getMeshId() const {
      return meshId;
   }

 private:
   std::string meshId;
   std::string textureId;
};