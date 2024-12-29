#include "TaskGraph.hpp"
#include "task/IRenderTask.hpp"

namespace tr {

// task render -> (dependency) compute
auto TaskGraph::resolveDependencies() -> void {

  for (const auto& node : taskNodes) {
    for (auto* dep : node.dependencies) {
      insertBarrier({.producer = dep->task, .consumer = node.task});
    }
  }
}

auto TaskGraph::insertBarrier(const BarrierConfig& barrierConfig) -> void {
  /*
     - figure out what the consumer's inputs are, and see if they are among the producer's
     outputs
     - if one is found, create a Barrier for it in the consumer
  */
  auto* consumer = barrierConfig.consumer;
  auto* provider = barrierConfig.producer;
  /*auto consumerResources = consumer->getResources();*/
  /*auto providerResources = provider->getResources();*/
  /**/
  /*auto consumerSet =*/
  /*    std::unordered_set<Resource>{consumerResources.begin(), consumerResources.end()};*/
  /**/
  /*for (const auto& producerResource : providerResources) {*/
  /*  if (consumerSet.find(producerResource) != consumerSet.end()) {*/
  /*    // Handle intersection, create a barrier with consumerResource and producerResource*/
  /*  }*/
  /*}*/
}

}

