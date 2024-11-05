#pragma once

#include "IPipeline.hpp"

namespace tr::gfx::pipe {
   class StaticModelPipeline : public IPipeline {
    public:
      StaticModelPipeline();
      ~StaticModelPipeline() override;
   };
}
