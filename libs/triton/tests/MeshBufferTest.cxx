#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <trompeloeil/mock.hpp>

#include <di.hpp>

#include "IBufferManager.hpp"
#include "as/StaticVertex.hpp"
#include "cm/RenderMeshData.hpp"
#include "vk/MeshBufferManager.hpp"
#include "GeometryGenerator.hpp"

namespace di = boost::di;

using trompeloeil::_;

class MockBufferManager : public trompeloeil::mock_interface<tr::IBufferManager> {
public:
  IMPLEMENT_MOCK6(createBuffer);
  IMPLEMENT_MOCK2(createGpuVertexBuffer);
  IMPLEMENT_MOCK2(createGpuIndexBuffer);
  IMPLEMENT_MOCK1(createIndirectBuffer);
  IMPLEMENT_MOCK2(resizeBuffer);
  IMPLEMENT_CONST_MOCK1(getBuffer);
  IMPLEMENT_MOCK5(addToBuffer);
};

// TEST_CASE("MeshBuffer Remove Mesh", "[mesh]") {
//   {
//     const auto geometryData = generateMesh(100, 300);
//     const auto geometryData2 = generateMesh(200, 600);
//     const auto geometryData3 = generateMesh(50, 150);

//     std::shared_ptr<MockBufferManager> mockBufferManager = std::make_shared<MockBufferManager>();

//     trompeloeil::sequence seq;

//     const auto vbh = 1;
//     const auto ibh = 2;

//     ALLOW_CALL(*mockBufferManager, createGpuVertexBuffer(204800,
//     "Buffer-name-Vertex")).RETURN(vbh); ALLOW_CALL(*mockBufferManager,
//     createGpuIndexBuffer(120960, "Buffer-name-Index")).RETURN(ibh);
//     ALLOW_CALL(*mockBufferManager, addToBuffer(_, vbh, 0, ibh, 0)).IN_SEQUENCE(seq);
//     ALLOW_CALL(*mockBufferManager, addToBuffer(_, vbh, 2000, ibh, 1200)).IN_SEQUENCE(seq);
//     ALLOW_CALL(*mockBufferManager, addToBuffer(_, vbh, 0, ibh, 0)).IN_SEQUENCE(seq);

//     const auto injector = di::make_injector(di::bind<tr::IBufferManager>.to(mockBufferManager),
//                                             di::bind<size_t>.to(sizeof(as::StaticVertex)),
//                                             di::bind<std::string_view>.to("name"));

//     auto meshBufferManager = injector.create<std::unique_ptr<tr::MeshBufferManager>>();

//     auto renderMeshDataList = std::vector<tr::RenderMeshData>{};

//     const auto result = meshBufferManager->addMesh(geometryData);
//     renderMeshDataList.push_back(tr::RenderMeshData{.handle = result});

//     const auto result2 = meshBufferManager->addMesh(geometryData2);
//     renderMeshDataList.push_back(tr::RenderMeshData{.handle = result2});

//     meshBufferManager->removeMesh(result);
//     renderMeshDataList.erase(renderMeshDataList.begin());

//     const auto result3 = meshBufferManager->addMesh(geometryData3);
//     renderMeshDataList.push_back(tr::RenderMeshData{.handle = result3});

//     const auto bufferEntries = meshBufferManager->getGpuBufferEntries(renderMeshDataList);

//     REQUIRE(bufferEntries.size() == 2);

//     REQUIRE(bufferEntries[0].indexCount == 600);
//     REQUIRE(bufferEntries[0].firstIndex == 300);
//     REQUIRE(bufferEntries[0].vertexOffset == 100);
//     REQUIRE(bufferEntries[0].instanceCount == 1);
//     REQUIRE(bufferEntries[0].firstInstance == 0);
//     REQUIRE(bufferEntries[0].padding == 0);

//     REQUIRE(bufferEntries[1].indexCount == 150);
//     REQUIRE(bufferEntries[1].firstIndex == 0);
//     REQUIRE(bufferEntries[1].vertexOffset == 0);
//     REQUIRE(bufferEntries[1].instanceCount == 1);
//     REQUIRE(bufferEntries[1].firstInstance == 0);
//     REQUIRE(bufferEntries[1].padding == 0);

//     meshBufferManager = nullptr;
//   }
// }

TEST_CASE("MeshBuffer Add Mesh", "[meshbuffer]") {

  {
    initLogger(spdlog::level::trace, spdlog::level::trace);

    const auto geometryData = generateMesh(100, 300);
    const auto geometryData2 = generateMesh(200, 600);

    std::shared_ptr<MockBufferManager> mBuffer = std::make_shared<MockBufferManager>();

    trompeloeil::sequence seq;

    const auto vbh = 1;
    const auto ibh = 2;

    ALLOW_CALL(*mBuffer, createGpuVertexBuffer(204800, "Buffer-name-Vertex")).RETURN(vbh);
    ALLOW_CALL(*mBuffer, createGpuIndexBuffer(120960, "Buffer-name-Index")).RETURN(ibh);
    ALLOW_CALL(*mBuffer, addToBuffer(_, vbh, 0, ibh, 0)).IN_SEQUENCE(seq);
    ALLOW_CALL(*mBuffer, addToBuffer(_, vbh, 2000, ibh, 1200)).IN_SEQUENCE(seq);

    const auto injector = di::make_injector(di::bind<tr::IBufferManager>.to(mBuffer),
                                            di::bind<size_t>.to(sizeof(as::StaticVertex)),
                                            di::bind<std::string_view>.to("name"));

    auto meshBufferManager2 = injector.create<std::unique_ptr<tr::MeshBufferManager>>();

    auto renderMeshDataList = std::vector<tr::RenderMeshData>{};

    const auto result = meshBufferManager2->addMesh(geometryData);
    renderMeshDataList.push_back(tr::RenderMeshData{.handle = result});

    const auto result2 = meshBufferManager2->addMesh(geometryData2);
    renderMeshDataList.push_back(tr::RenderMeshData{.handle = result2});

    const auto bufferEntries = meshBufferManager2->getGpuBufferEntries(renderMeshDataList);

    REQUIRE(bufferEntries.size() == 2);

    REQUIRE(bufferEntries[0].indexCount == 300);
    REQUIRE(bufferEntries[0].firstIndex == 0);
    REQUIRE(bufferEntries[0].vertexOffset == 0);
    REQUIRE(bufferEntries[0].instanceCount == 1);
    REQUIRE(bufferEntries[0].firstInstance == 0);
    REQUIRE(bufferEntries[0].padding == 0);

    REQUIRE(bufferEntries[1].indexCount == 600);
    REQUIRE(bufferEntries[1].firstIndex == 300);
    REQUIRE(bufferEntries[1].vertexOffset == 100);
    REQUIRE(bufferEntries[1].instanceCount == 1);
    REQUIRE(bufferEntries[1].firstInstance == 0);
    REQUIRE(bufferEntries[1].padding == 0);
    meshBufferManager2 = nullptr;
  }
}
