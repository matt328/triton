#pragma once

#include <bitset>

namespace tr::cm::sdf {

   struct CellData {
      const glm::ivec3 offset;
      const uint8_t caseCode;
      const int8_t equivalenceClassNumber;
      const std::vector<glm::vec3> vertices;

      [[nodiscard]] auto toString() const -> std::string {
         const auto& hex = fmt::format("{:02x}", caseCode);
         auto bitsetCaseCode = std::bitset<8>{caseCode};
         auto str = fmt::format("Cell Position: ({0}, {1}, {2}), Case Code {3} (0x{4}) {5}, "
                                "Equivalence Class Number: {6}",
                                offset.x,
                                offset.y,
                                offset.z,
                                caseCode,
                                hex,
                                bitsetCaseCode.to_string(),
                                equivalenceClassNumber);
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
         activeCubes.emplace_back(std::forward<Args>(args)...);
      }

      [[nodiscard]] auto getActiveCubePositions() const -> const std::vector<CellData>& {
         return activeCubes;
      }

    private:
      VoxelDebugger() = default;
      ~VoxelDebugger() = default;

      std::vector<CellData> activeCubes;
   };

} // namespace tr::cm::sdf
