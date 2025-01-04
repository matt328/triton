#pragma once
#include <gp/AnimationFactory.hpp>

namespace tr {

class AnimationSystem {
public:
  explicit AnimationSystem(std::shared_ptr<AnimationFactory> newAnimationFactory);
  ~AnimationSystem() = default;

  AnimationSystem(const AnimationSystem&) = delete;
  AnimationSystem(AnimationSystem&&) = delete;
  auto operator=(const AnimationSystem&) -> AnimationSystem& = delete;
  auto operator=(AnimationSystem&&) -> AnimationSystem& = delete;

  auto update(entt::registry& registry) const -> void;

private:
  std::shared_ptr<AnimationFactory> animationFactory;
};

}
