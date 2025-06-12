#pragma once

namespace tr {

template <typename T>
class DirectedGraph {
public:
  auto addNode(const T& node) -> void {
    adjacencyList[node];
  }

  auto addEdge(const T& from, const T& to) -> void {
    Log.trace("addEdge() from={}, to={}", from, to);
    addNode(from);
    addNode(to);
    adjacencyList[from].insert(to);
    inDegree[to]++;
  }

  auto topologicalSort() const -> std::vector<T> {
    auto localInDegree = inDegree;
    auto zeroInDegree = std::queue<T>{};

    for (const auto& [node, _] : adjacencyList) {
      if (localInDegree[node] == 0) {
        zeroInDegree.push(node);
      }
    }

    auto result = std::vector<T>{};
    while (!zeroInDegree.empty()) {
      auto node = zeroInDegree.front();
      zeroInDegree.pop();
      result.push_back(node);

      for (const auto& neighbor : adjacencyList.at(node)) {
        if (--localInDegree[neighbor] == 0) {
          zeroInDegree.push(neighbor);
        }
      }
    }

    if (result.size() != adjacencyList.size()) {
      throw std::runtime_error("Cycle detected in graph");
    }

    return result;
  }

  auto hasEdge(const T& from, const T& to) -> bool {
    return adjacencyList.contains(from) && adjacencyList.at(from).contains(to);
  }

private:
  std::unordered_map<T, std::unordered_set<T>> adjacencyList;
  std::unordered_map<T, size_t> inDegree;
};

}
