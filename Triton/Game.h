#pragma once

class Game {
 public:
   Game() = default;
   ~Game() = default;

   void update(double t, float dt);
   void blendState(double alpha);

};
