#pragma once

namespace tr::gp {
   class AnimationFactory;
}

namespace tr::gp::sys::AnimationSystem {
   void update(entt::registry& registry, AnimationFactory& animationFactory);
}