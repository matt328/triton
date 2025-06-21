#pragma once

#include "r3/graph/barriers/AccessMode.hpp"
namespace tr {

struct LastImageUse {
  AccessMode accessMode;
  vk::AccessFlags2 access;
  vk::PipelineStageFlags2 stage;
  vk::ImageLayout layout;
};

}
