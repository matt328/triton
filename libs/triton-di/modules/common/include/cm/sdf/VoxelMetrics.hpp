#pragma once

#include "as/Vertex.hpp"
namespace tr::cm::sdf {

   static constexpr uint8_t BitCount = 8;

   struct CellData {
      const glm::ivec3 offset;
      const uint8_t caseCode;
      const int8_t equivalenceClassNumber;
      const std::vector<as::Vertex> vertices;

      [[nodiscard]] auto toString() const -> std::string {
         const auto& hex = std::format("{:02x}", caseCode);

         auto bitsetCaseCode = std::bitset<BitCount>{caseCode};

         auto str = std::format("Cell Position: ({0}, {1}, {2}), Case Code {3} (0x{4}) {5}, "
                                "Equivalence Class Number: {6}, vertices size: {7}",
                                offset.x,
                                offset.y,
                                offset.z,
                                caseCode,
                                hex,
                                bitsetCaseCode.to_string(),
                                equivalenceClassNumber,
                                vertices.size());
         return str;
      }
   };

   class VoxelDebugger {
    public:
      VoxelDebugger(VoxelDebugger&&) = delete;
      auto operator=(VoxelDebugger&&) -> VoxelDebugger& = delete;
      VoxelDebugger(const VoxelDebugger&) = delete;
      auto operator=(const VoxelDebugger&) -> VoxelDebugger& = delete;

      static auto getInstance() -> VoxelDebugger& {
         static VoxelDebugger instance;
         return instance;
      }

      template <typename... Args>
      void addActiveCube(Args&&... args) {
         auto argsTuple = std::make_tuple(std::forward<Args>(args)...);
         const auto cellPosition = std::get<0>(argsTuple);

         const auto cellName =
             std::format("[{0}, {1}, {2}]", cellPosition.x, cellPosition.y, cellPosition.z);

         activeCubes.emplace(std::piecewise_construct,
                             std::forward_as_tuple(cellName),
                             std::forward_as_tuple(std::forward<Args>(args)...));
      }

      [[nodiscard]] auto getActiveCubePositions() const -> const std::map<std::string, CellData>& {
         return activeCubes;
      }

    private:
      VoxelDebugger() = default;
      ~VoxelDebugger() = default;

      std::map<std::string, CellData> activeCubes;
   };

} // namespace tr::cm::sdf
