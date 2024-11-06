#pragma once

class IBus {
 public:
   virtual void sendMessage() = 0;
   IBus() = default;
   virtual ~IBus() = default;
};
