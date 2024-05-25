#pragma once

namespace tr::gfx::geo {
   class AnimationFactory;
}

namespace tr::gp::ecs::AnimationSystem {
   void update(entt::registry& registry, gfx::geo::AnimationFactory& animationFactory);
}