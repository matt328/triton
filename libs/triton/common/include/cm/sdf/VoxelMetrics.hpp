#pragma once

namespace tr::cm::sdf {

   struct CellData {
      const glm::ivec3 offset;
      const uint8_t caseCode;
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

} // namespace tr::gfx
