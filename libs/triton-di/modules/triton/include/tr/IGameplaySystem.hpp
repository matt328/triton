#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/RenderData.hpp"
#include <spdlog/fmt/bundled/core.h>

namespace tr::gp {

   using RenderDataTransferHandler = std::function<void(cm::gpu::RenderData&)>;

   struct AnimatedModelData {
      std::string modelFilename;
      std::string skeletonFilename;
      std::string animationFilename;
   };

   class IGameplaySystem {
    public:
      explicit IGameplaySystem() = default;
      virtual ~IGameplaySystem() = default;

      IGameplaySystem(const IGameplaySystem&) = default;
      IGameplaySystem(IGameplaySystem&&) = delete;
      auto operator=(const IGameplaySystem&) -> IGameplaySystem& = default;
      auto operator=(IGameplaySystem&&) -> IGameplaySystem& = delete;

      virtual void update() = 0;
      virtual void fixedUpdate() = 0;

      virtual void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) = 0;

      virtual auto createStaticModelEntity(std::string filename, std::string_view entityName)
          -> void = 0;
      virtual auto createAnimatedModelEntity(const AnimatedModelData& modelData)
          -> cm::EntityType = 0;
      virtual auto createTerrain() -> cm::EntityType = 0;
      virtual auto createDefaultCamera() -> void = 0;
      virtual auto createTestEntity(std::string_view name) -> void = 0;
   };
}

// Specialize fmt::formatter for AnimatedModelData
template <>
struct fmt::formatter<tr::gp::AnimatedModelData> {
   constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const tr::gp::AnimatedModelData& data, FormatContext& ctx) -> decltype(ctx.out()) {
      return fmt::format_to(
          ctx.out(),
          "AnimatedModelData(modelFilename: '{}', skeletonFilename: '{}', animationFilename: '{}')",
          data.modelFilename,
          data.skeletonFilename,
          data.animationFilename);
   }
};