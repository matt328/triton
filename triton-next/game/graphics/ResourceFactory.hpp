#pragma once

namespace Triton {
   class ResourceFactory {
    public:
      ResourceFactory() = default;
      virtual ~ResourceFactory() = default;

      ResourceFactory(const ResourceFactory&) = default;
      ResourceFactory(ResourceFactory&&) = delete;
      ResourceFactory& operator=(const ResourceFactory&) = default;
      ResourceFactory& operator=(ResourceFactory&&) = delete;

      virtual std::string createMesh(const std::string_view& filename) = 0;
      virtual uint32_t createTexture(const std::string_view& filename) = 0;
   };
}
