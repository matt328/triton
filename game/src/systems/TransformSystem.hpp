#pragma once

#include "InputSystem.hpp"
#include "ActionMapper.hpp"

class TransformSystem {
 public:
   explicit TransformSystem();
   TransformSystem(const TransformSystem&) = default;
   TransformSystem(TransformSystem&&) = delete;
   TransformSystem& operator=(const TransformSystem&) = default;
   TransformSystem& operator=(TransformSystem&&) = delete;
   ~TransformSystem() = default;

   void update(entt::registry& registry) const;

   void handleAction(const Input::Action& action);

   int recieve(const int& i) {
      return i;
   }
};
