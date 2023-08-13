#pragma once

struct Renderable {
   explicit Renderable(const std::string_view& meshId, const std::string_view& textureId) :
       meshId(meshId.data()), textureId(textureId.data()) {
   }

   [[nodiscard]] std::string getMeshId() const {
      return meshId;
   }

   [[nodiscard]] std::string getTextureId() const {
      return textureId;
   }

 private:
   std::string meshId;
   std::string textureId;
};