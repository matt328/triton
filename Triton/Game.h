#pragma once

class Game {
public:
   Game() = default;
  ~Game() = default;

   void update(float t, float dt);
  void blendState(double alpha);

};
