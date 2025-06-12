#include "PassGraphBuilder.hpp"

namespace tr {
auto PassGraphBuilder::build(const std::unordered_map<PassId, std::unique_ptr<IRenderPass>>& passes)
    const -> DirectedGraph<PassId> {

  DirectedGraph<PassId> graph;

  std::unordered_map<ImageAlias, PassId> lastWriterImages;
  std::unordered_map<ImageAlias, std::vector<PassId>> pendingReadersImages;

  std::unordered_map<BufferAlias, PassId> lastWriterBuffers;
  std::unordered_map<BufferAlias, std::vector<PassId>> pendingReadersBuffers;

  for (const auto& [id, pass] : passes) {
    const auto& info = pass->getGraphInfo();

    auto processUsages =
        [&](const auto& usages, bool isWrite, auto& lastWriterMap, auto& pendingReadersMap) {
          for (const auto& usage : usages) {
            const auto& alias = usage.alias;

            if (isWrite) {
              // Connect to all prior readers (deferred edge)
              if (pendingReadersMap.contains(alias)) {
                for (PassId readerId : pendingReadersMap[alias]) {
                  graph.addEdge(id, readerId);
                }
                pendingReadersMap.erase(alias);
              }

              // Connect to last writer
              if (lastWriterMap.contains(alias)) {
                Log.trace("addingEdge for lastWriter");
                graph.addEdge(id, lastWriterMap[alias]);
              }

              lastWriterMap[alias] = id;

            } else {
              // Connect to last writer
              if (lastWriterMap.contains(alias)) {
                Log.trace("addingEdge for last write #2");
                graph.addEdge(id, lastWriterMap[alias]);
              } else {
                // No writer yet — defer connection
                pendingReadersMap[alias].push_back(id);
              }
            }
          }
        };

    processUsages(info.imageReads, false, lastWriterImages, pendingReadersImages);
    processUsages(info.imageWrites, true, lastWriterImages, pendingReadersImages);
    processUsages(info.bufferReads, false, lastWriterBuffers, pendingReadersBuffers);
    processUsages(info.bufferWrites, true, lastWriterBuffers, pendingReadersBuffers);
  }

  return graph;
}

}
