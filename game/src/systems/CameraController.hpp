#pragma once

#include "Action.hpp"

class CameraController {
 public:
   void moveForward(Input::Action action){};
   void setActiveCamera(entt::entity entity);
};