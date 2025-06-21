#include "BarrierPrecursorGenerator.hpp"

namespace tr {

auto BarrierPrecursorGenerator::build(const std::vector<std::unique_ptr<IRenderPass>>& passes)
    -> BarrierPrecursorPlan {
  BarrierPrecursorPlan result{};

  for (const auto& pass : passes) {
    const auto& passId = pass->getId();
    const auto info = pass->getGraphInfo();

    // Image Reads
    for (const auto& read : info.imageReads) {
      result.imagePrecursors[passId].push_back(ImageBarrierPrecursor{
          .alias = read.alias,
          .accessMode = AccessMode::Read,
          .accessFlags = read.accessFlags,
          .stageFlags = read.stageFlags,
          .layout = read.layout,
          .aspectFlags = read.aspectFlags,
      });
    }

    // Image Writes
    for (const auto& write : info.imageWrites) {
      result.imagePrecursors[passId].push_back(ImageBarrierPrecursor{
          .alias = write.alias,
          .accessMode = AccessMode::Write,
          .accessFlags = write.accessFlags,
          .stageFlags = write.stageFlags,
          .layout = write.layout,
          .aspectFlags = write.aspectFlags,
      });
    }

    // Buffer Reads
    for (const auto& read : info.bufferReads) {
      result.bufferPrecursors[passId].push_back(BufferBarrierPrecursor{
          .alias = read.alias,
          .accessMode = AccessMode::Read,
          .accessFlags = read.accessFlags,
          .stageFlags = read.stageFlags,
      });
    }

    // Buffer Writes
    for (const auto& write : info.bufferWrites) {
      result.bufferPrecursors[passId].push_back(BufferBarrierPrecursor{
          .alias = write.alias,
          .accessMode = AccessMode::Write,
          .accessFlags = write.accessFlags,
          .stageFlags = write.stageFlags,
      });
    }
  }

  return result;
}

}
