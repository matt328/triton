#pragma once

struct Renderable {
   explicit Renderable(const std::string_view& meshId, const uint32_t newTextureId) :
       meshId(meshId.data()), textureId(newTextureId) {
   }

   [[nodiscard]] std::string getMeshId() const {
      return meshId;
   }

   [[nodiscard]] uint32_t getTextureId() const {
      return textureId;
   }

 private:
   std::string meshId;
   uint32_t textureId;
};
