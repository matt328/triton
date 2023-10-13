#pragma once

class IResourceFactory {
 public:
   IResourceFactory() = default;
   IResourceFactory(const IResourceFactory&) = default;
   IResourceFactory(IResourceFactory&&) = delete;
   IResourceFactory& operator=(const IResourceFactory&) = default;
   IResourceFactory& operator=(IResourceFactory&&) = delete;

   virtual ~IResourceFactory() = default;

   virtual std::string createMesh(const std::string_view& filename) = 0;
   virtual uint32_t createTexture(const std::string_view& filename) = 0;
};
