#pragma once
#include "AbstractPipeline.h"
class DefaultPipeline final : public AbstractPipeline {
 public:
   DefaultPipeline();
   ~DefaultPipeline() override;
};
