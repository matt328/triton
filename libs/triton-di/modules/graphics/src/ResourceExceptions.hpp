#pragma once

#include "BaseException.hpp"

namespace tr::gfx {
   class ResourceUploadException final : public BaseException {
    public:
      using BaseException::BaseException;
   };

   class ResourceCreateException final : public tr::BaseException {
    public:
      using BaseException::BaseException;
   };
}