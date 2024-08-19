#include "as/gltf/ModelLoader.hpp"

#include "catch2/matchers/catch_matchers.hpp"
#include <catch2/catch_test_macros.hpp>

class MockGltfLoader final : public trompeloeil::mock_interface<tr::as::GltfFileLoader> {
 public:
   IMPLEMENT_MOCK4(loadFromFile);
};

TEST_CASE("ModelLoader is tested", "[ModelLoader]") {
   SECTION("loader warning throws an exception") {
      MockGltfLoader mockLoader;

      using trompeloeil::_;

      REQUIRE_CALL(mockLoader, loadFromFile(_, _, _, _))
          .LR_SIDE_EFFECT(*_3 = "warning message")
          .RETURN(false);

      const auto loader = tr::as::gltf::ModelLoader{};

      REQUIRE_THROWS_WITH(loader.load(&mockLoader, "some_path"), "warning message");
   }

   SECTION("loader error throws an exception") {
      MockGltfLoader mockLoader;

      using trompeloeil::_;

      REQUIRE_CALL(mockLoader, loadFromFile(_, _, _, _))
          .LR_SIDE_EFFECT(*_2 = "error message")
          .RETURN(false);

      const auto loader = tr::as::gltf::ModelLoader{};

      REQUIRE_THROWS_WITH(loader.load(&mockLoader, "some_path"), "error message");
   }

   SECTION("loader returning false throws an exception") {
      MockGltfLoader mockLoader;

      using trompeloeil::_;

      REQUIRE_CALL(mockLoader, loadFromFile(_, _, _, _)).RETURN(false);

      const auto loader = tr::as::gltf::ModelLoader{};

      REQUIRE_THROWS_WITH(loader.load(&mockLoader, "some_path"), "Failed to parse glTF file");
   }
}