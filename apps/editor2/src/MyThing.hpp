#pragma once

#include "cm/services/IThing.hpp"

class MyThing : public IThing {
   void sayHello() override {
      std::cout << "Hello from my thing" << '\n';
   }
};
