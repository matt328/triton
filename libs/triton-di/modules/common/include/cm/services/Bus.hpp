#pragma once

#include "IBus.hpp"

class Bus : public IBus {
 public:
   Bus() = default;
   ~Bus();

   void sendMessage() override {
   }
};
