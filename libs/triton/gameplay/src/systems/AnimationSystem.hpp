#pragma once

namespace tr::gp {
   class AnimationFactory;
}

namespace tr::gp::ecs::AnimationSystem {
   void update(entt::registry& registry, AnimationFactory& animationFactory);
}