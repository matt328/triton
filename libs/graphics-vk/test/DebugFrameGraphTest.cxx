#include "r3/graph/PassGraphBuilder.hpp"
#include "r3/render-pass/IRenderPass.hpp"

#include <catch2/catch_session.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>

namespace tr {

struct MockRenderPass : public IRenderPass {
  PassId id;
  PassGraphInfo info;

  MockRenderPass(PassId pid, tr::PassGraphInfo pinfo) : id(pid), info(std::move(pinfo)) {
  }

  auto getId() const -> PassId override {
    return id;
  }

  auto getGraphInfo() const -> PassGraphInfo override {
    return info;
  }

  auto execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void override {};
  auto registerDispatchContext(Handle<IDispatchContext> handle) -> void override {};
};

TEST_CASE("PassGraphBuilder builds correct dependency graph", "[PassGraphBuilder]") {
  initLogger(spdlog::level::trace, spdlog::level::trace);
  // Pass 1 writes image A
  auto pass1 = std::make_unique<MockRenderPass>(
      PassId::Forward,
      PassGraphInfo{
          .imageWrites = {ImageUsageInfo{.alias = ImageAlias::GeometryColorImage}},
      });

  // Pass 2 reads image A
  auto pass2 = std::make_unique<MockRenderPass>(
      PassId::Composition,
      PassGraphInfo{
          .imageReads = {ImageUsageInfo{.alias = ImageAlias::GeometryColorImage}},
      });

  // Pass 3 writes buffer B
  auto pass3 = std::make_unique<MockRenderPass>(
      PassId::Culling,
      PassGraphInfo{
          .bufferWrites = {BufferUsageInfo{.alias = BufferAlias::IndirectCommand}},
      });

  // Pass 4 reads buffer B
  auto pass4 = std::make_unique<MockRenderPass>(
      PassId::PostProcessing,
      PassGraphInfo{
          .bufferReads = {BufferUsageInfo{.alias = BufferAlias::IndirectCommand}},
      });

  std::unordered_map<PassId, std::unique_ptr<IRenderPass>> passes;
  passes.emplace(PassId::Forward, std::move(pass1));
  passes.emplace(PassId::Composition, std::move(pass2));
  passes.emplace(PassId::Culling, std::move(pass3));
  passes.emplace(PassId::PostProcessing, std::move(pass4));

  // Act
  auto graph = PassGraphBuilder{}.build(passes);

  // Assert
  REQUIRE(graph.hasEdge(PassId::Forward,
                        PassId::Composition)); // Pass 1 -> Pass 2 (image A write->read)

  REQUIRE(graph.hasEdge(PassId::Culling,
                        PassId::PostProcessing)); // Pass 3 -> Pass 4 (buffer B write->read)

  // Negative checks (these edges shouldn't exist)
  REQUIRE_FALSE(graph.hasEdge(PassId::Composition, PassId::Forward));
  REQUIRE_FALSE(graph.hasEdge(PassId::PostProcessing, PassId::Culling));
  REQUIRE_FALSE(graph.hasEdge(PassId::Forward, PassId::PostProcessing));
  REQUIRE_FALSE(graph.hasEdge(PassId::Culling, PassId::Composition));
}
}
