#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <trompeloeil/mock.hpp>

#include "mem/ArenaBuffer.hpp"
#include "IBufferManager.hpp"
#include "geo/GeometryData.hpp"
#include "mem/BufferRegion.hpp"

using trompeloeil::_;

class MockBufferManager : public trompeloeil::mock_interface<tr::IBufferManager> {
public:
  IMPLEMENT_MOCK6(createBuffer);
  IMPLEMENT_MOCK2(createGpuVertexBuffer);
  IMPLEMENT_MOCK2(createGpuIndexBuffer);
  IMPLEMENT_MOCK1(createIndirectBuffer);
  IMPLEMENT_MOCK2(resizeBuffer);
  IMPLEMENT_MOCK3(removeData);
  IMPLEMENT_CONST_MOCK1(getBuffer);
  IMPLEMENT_MOCK5(addToBuffer);
  IMPLEMENT_MOCK4(addToSingleBuffer);
};

TEST_CASE("ArenaBuffer insertData with merge with previous neighbor", "[arenabuffer]") {
  std::shared_ptr<MockBufferManager> mockBuffer = std::make_shared<MockBufferManager>();

  const auto ci =
      tr::ArenaBufferCreateInfo{.newItemStride = 1, .initialBufferSize = 100, .bufferName = "Test"};

  size_t dataSize = 5;
  auto data = std::vector<char>(dataSize);

  trompeloeil::sequence seq;

  auto bufferHandle = 1;
  REQUIRE_CALL(*mockBuffer, createGpuVertexBuffer(100, "Buffer-Test-Vertex")).RETURN(bufferHandle);
  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 0))
      .IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 5))
      .IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 10))
      .IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer, removeData(bufferHandle, 0, dataSize)).IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, removeData(bufferHandle, 5, dataSize)).IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, removeData(bufferHandle, 10, dataSize)).IN_SEQUENCE(seq);

  auto arenaBuffer = std::make_shared<tr::ArenaBuffer>(mockBuffer, ci);

  auto bufferRegion1 = arenaBuffer->insertData(data.data(), dataSize);
  auto bufferRegion2 = arenaBuffer->insertData(data.data(), dataSize);
  auto bufferRegion3 = arenaBuffer->insertData(data.data(), dataSize);

  arenaBuffer->removeData(bufferRegion1);
  arenaBuffer->removeData(bufferRegion2);
  arenaBuffer->removeData(bufferRegion3);
}

TEST_CASE("ArenaBuffer insertData with merge with next neighbor", "[arenabuffer]") {
  std::shared_ptr<MockBufferManager> mockBuffer = std::make_shared<MockBufferManager>();

  const auto ci =
      tr::ArenaBufferCreateInfo{.newItemStride = 1, .initialBufferSize = 100, .bufferName = "Test"};

  size_t dataSize = 5;
  auto data = std::vector<char>(dataSize);

  trompeloeil::sequence seq;

  auto bufferHandle = 1;
  REQUIRE_CALL(*mockBuffer, createGpuVertexBuffer(100, "Buffer-Test-Vertex")).RETURN(bufferHandle);
  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 0))
      .IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 5))
      .IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 10))
      .IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer, removeData(bufferHandle, 10, dataSize)).IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, removeData(bufferHandle, 5, dataSize)).IN_SEQUENCE(seq);
  REQUIRE_CALL(*mockBuffer, removeData(bufferHandle, 0, dataSize)).IN_SEQUENCE(seq);

  auto arenaBuffer = std::make_shared<tr::ArenaBuffer>(mockBuffer, ci);

  auto bufferRegion1 = arenaBuffer->insertData(data.data(), dataSize);
  auto bufferRegion2 = arenaBuffer->insertData(data.data(), dataSize);
  auto bufferRegion3 = arenaBuffer->insertData(data.data(), dataSize);

  arenaBuffer->removeData(bufferRegion3);
  arenaBuffer->removeData(bufferRegion2);
  arenaBuffer->removeData(bufferRegion1);
}

TEST_CASE("ArenaBuffer insertData with buffer resize", "[arenabuffer]") {
  std::shared_ptr<MockBufferManager> mockBuffer = std::make_shared<MockBufferManager>();

  const auto ci =
      tr::ArenaBufferCreateInfo{.newItemStride = 1, .initialBufferSize = 100, .bufferName = "Test"};

  size_t dataSize = 100;
  auto data = std::vector<char>(dataSize);

  size_t smallDataSize = 5;
  auto smallData = std::vector<char>(smallDataSize);

  trompeloeil::sequence seq;

  auto bufferHandle = 1;
  auto newBufferHandle = 2;
  REQUIRE_CALL(*mockBuffer, createGpuVertexBuffer(100, "Buffer-Test-Vertex")).RETURN(bufferHandle);

  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 0))
      .IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer, resizeBuffer(bufferHandle, 200))
      .RETURN(newBufferHandle)
      .IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer,
               addToSingleBuffer(smallData.data(), smallDataSize, newBufferHandle, 100))
      .IN_SEQUENCE(seq);

  auto arenaBuffer = std::make_shared<tr::ArenaBuffer>(mockBuffer, ci);

  auto bufferRegion1 = arenaBuffer->insertData(data.data(), dataSize);
  auto bufferRegion2 = arenaBuffer->insertData(smallData.data(), smallDataSize);

  REQUIRE(bufferRegion1.offset == 0);
  REQUIRE(bufferRegion1.size == dataSize);

  REQUIRE(bufferRegion2.offset == 100);
  REQUIRE(bufferRegion2.size == smallDataSize);
}

TEST_CASE("ArenaBuffer insertData into emptied regions", "[arenabuffer]") {
  std::shared_ptr<MockBufferManager> mockBuffer = std::make_shared<MockBufferManager>();

  const auto ci =
      tr::ArenaBufferCreateInfo{.newItemStride = 1, .initialBufferSize = 100, .bufferName = "Test"};

  size_t dataSize = 10;
  auto data = std::vector<char>(dataSize);

  size_t smallDataSize = 5;
  auto smallData = std::vector<char>(smallDataSize);

  trompeloeil::sequence seq;

  auto bufferHandle = 1;
  REQUIRE_CALL(*mockBuffer, createGpuVertexBuffer(100, "Buffer-Test-Vertex")).RETURN(bufferHandle);

  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 0))
      .IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 10))
      .IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer, removeData(bufferHandle, 0, dataSize)).IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(smallData.data(), smallDataSize, bufferHandle, 0))
      .IN_SEQUENCE(seq);

  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(smallData.data(), smallDataSize, bufferHandle, 5))
      .IN_SEQUENCE(seq);

  auto arenaBuffer = std::make_shared<tr::ArenaBuffer>(mockBuffer, ci);

  auto bufferRegion1 = arenaBuffer->insertData(data.data(), dataSize);
  auto bufferRegion2 = arenaBuffer->insertData(data.data(), dataSize);

  arenaBuffer->removeData(bufferRegion1);

  auto bufferRegion3 = arenaBuffer->insertData(smallData.data(), smallDataSize);
  auto bufferRegion4 = arenaBuffer->insertData(smallData.data(), smallDataSize);

  REQUIRE(bufferRegion1.offset == 0);
  REQUIRE(bufferRegion1.size == dataSize);

  REQUIRE(bufferRegion2.offset == 10);
  REQUIRE(bufferRegion2.size == dataSize);

  REQUIRE(bufferRegion3.offset == 0);
  REQUIRE(bufferRegion3.size == smallDataSize);

  REQUIRE(bufferRegion4.offset == 5);
  REQUIRE(bufferRegion4.size == smallDataSize);
}

TEST_CASE("ArenaBuffer insertData", "[arenabuffer]") {
  std::shared_ptr<MockBufferManager> mockBuffer = std::make_shared<MockBufferManager>();

  const auto ci =
      tr::ArenaBufferCreateInfo{.newItemStride = 1, .initialBufferSize = 100, .bufferName = "Test"};

  size_t dataSize = 10;
  auto data = std::vector<char>(dataSize);

  auto bufferHandle = 1;
  REQUIRE_CALL(*mockBuffer, createGpuVertexBuffer(100, "Buffer-Test-Vertex")).RETURN(bufferHandle);
  REQUIRE_CALL(*mockBuffer, addToSingleBuffer(data.data(), dataSize, bufferHandle, 0));

  auto arenaBuffer = std::make_shared<tr::ArenaBuffer>(mockBuffer, ci);

  auto bufferRegion1 = arenaBuffer->insertData(data.data(), dataSize);

  REQUIRE(bufferRegion1.size == dataSize);
  REQUIRE(bufferRegion1.offset == 0);
}

TEST_CASE("ArenaBuffer Construction", "[arenabuffer]") {
  std::shared_ptr<MockBufferManager> mockBufferManager = std::make_shared<MockBufferManager>();

  const auto ci =
      tr::ArenaBufferCreateInfo{.newItemStride = 5, .initialBufferSize = 100, .bufferName = "Test"};

  auto bufferHandle = 1;
  REQUIRE_CALL(*mockBufferManager, createGpuVertexBuffer(100, "Buffer-Test-Vertex"))
      .RETURN(bufferHandle);

  auto arenaBuffer = std::make_shared<tr::ArenaBuffer>(mockBufferManager, ci);

  REQUIRE(arenaBuffer != nullptr);
}
