#pragma once

class IThing {
 public:
   virtual void sayHello() = 0;
   virtual ~IThing() = default;
};
