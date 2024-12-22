#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/RenderData.hpp"
#include <spdlog/fmt/bundled/core.h>

namespace tr {

using RenderDataTransferHandler = std::function<void(RenderData&)>;

struct AnimatedModelData {
   std::string modelFilename;
   std::string skeletonFilename;
   std::string animationFilename;
   std::optional<std::string> entityName = std::nullopt;
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
   virtual auto createAnimatedModelEntity(const AnimatedModelData& modelData) -> void = 0;
   virtual auto createTerrain() -> void = 0;
   virtual auto createDefaultCamera() -> void = 0;
   virtual auto createTestEntity(std::string_view name) -> void = 0;
};
}

// Specialize fmt::formatter for AnimatedModelData
template <>
struct fmt::formatter<tr::AnimatedModelData> {
   constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const tr::AnimatedModelData& data, FormatContext& ctx) -> decltype(ctx.out()) {
      return fmt::format_to(
          ctx.out(),
          "AnimatedModelData(modelFilename: '{}', skeletonFilename: '{}', animationFilename: '{}')",
          data.modelFilename,
          data.skeletonFilename,
          data.animationFilename);
   }
};

