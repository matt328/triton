#pragma once

#include "ResourceFactory.hpp"

namespace Triton {
   class DefaultResourceFactory : public ResourceFactory {
    public:
      DefaultResourceFactory(std::filesystem::path rootPath);
      ~DefaultResourceFactory() override;

      DefaultResourceFactory(const DefaultResourceFactory&) = default;
      DefaultResourceFactory(DefaultResourceFactory&&) = delete;
      DefaultResourceFactory& operator=(const DefaultResourceFactory&) = default;
      DefaultResourceFactory& operator=(DefaultResourceFactory&&) = delete;

      std::string createMesh(const std::string_view& filename) override;
      uint32_t createTexture(const std::string_view& filename) override;
   };

}