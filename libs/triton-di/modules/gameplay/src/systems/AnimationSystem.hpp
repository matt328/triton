#pragma once
#include <gp/AnimationFactory.hpp>
#include <gp/Registry.hpp>

namespace tr {

   class AnimationSystem {
    public:
      explicit AnimationSystem(std::shared_ptr<Registry> newRegistry,
                               std::shared_ptr<AnimationFactory> newAnimationFactory);
      ~AnimationSystem() = default;

      AnimationSystem(const AnimationSystem&) = delete;
      AnimationSystem(AnimationSystem&&) = delete;
      auto operator=(const AnimationSystem&) -> AnimationSystem& = delete;
      auto operator=(AnimationSystem&&) -> AnimationSystem& = delete;

      auto update() const -> void;

    private:
      std::shared_ptr<Registry> registry;
      std::shared_ptr<AnimationFactory> animationFactory;
   };

}
