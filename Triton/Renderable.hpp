#pragma once

struct Renderable {
   explicit Renderable(const std::string_view& meshId) : meshId(meshId.data()) {
   }

   std::string getMeshId() const {
      return meshId;
   }

 private:
   std::string meshId;
};