#pragma once

#include "r3/graph/ResourceAliases.hpp"
namespace tr {

struct ImageBarrierData {
  vk::ImageMemoryBarrier2 imageBarrier;
  ImageAlias alias;
};

struct BufferBarrierData {
  vk::BufferMemoryBarrier2 bufferBarrier;
  BufferAliasVariant alias;
};

}
