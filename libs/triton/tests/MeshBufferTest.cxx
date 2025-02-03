#include <catch2/catch_test_macros.hpp>
#include <di.hpp>

#include "IBufferManager.hpp"
#include "MockBufferManager.hpp"
#include "cm/RenderMeshData.hpp"
#include "vk/MeshBufferManager.hpp"
#include "GeometryGenerator.hpp"

namespace di = boost::di;

TEST_CASE("MeshBuffer", "[meshbuffer]") {

  initLogger(spdlog::level::trace, spdlog::level::trace);

  size_t size = 40;
  std::string_view name = "name";
  const auto injector = di::make_injector(di::bind<tr::IBufferManager>.to<tr::MockBufferManager>(),
                                          di::bind<size_t>.to(size),
                                          di::bind<std::string_view>.to(name));

  auto meshBufferManager = injector.create<std::shared_ptr<tr::MeshBufferManager>>();

  const auto geometryData = generateMesh(100);
  const auto geometryData2 = generateMesh(200);

  auto renderMeshDataList = std::vector<tr::RenderMeshData>{};

  const auto result = meshBufferManager->addMesh(geometryData);
  renderMeshDataList.push_back(tr::RenderMeshData{.handle = result});

  const auto result2 = meshBufferManager->addMesh(geometryData2);
  renderMeshDataList.push_back(tr::RenderMeshData{.handle = result2});

  const auto bufferEntries = meshBufferManager->getGpuBufferEntries(renderMeshDataList);

  REQUIRE(bufferEntries.size() == 2);

  REQUIRE(bufferEntries[0].indexCount == 400);
  REQUIRE(bufferEntries[0].firstIndex == 0);
  REQUIRE(bufferEntries[0].vertexOffset == 0);
  REQUIRE(bufferEntries[0].instanceCount == 1);
  REQUIRE(bufferEntries[0].firstInstance == 0);
  REQUIRE(bufferEntries[0].padding == 0);

  REQUIRE(bufferEntries[1].indexCount == 800);
  REQUIRE(bufferEntries[1].firstIndex == 400);
  REQUIRE(bufferEntries[1].vertexOffset == 100);
  REQUIRE(bufferEntries[1].instanceCount == 1);
  REQUIRE(bufferEntries[1].firstInstance == 0);
  REQUIRE(bufferEntries[1].padding == 0);
}

TEST_CASE("MeshBuffer#2", "[mesh]") {
  REQUIRE(8 == 8);
}
