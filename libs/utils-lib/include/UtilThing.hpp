#pragma once

namespace util {
   class UtilThing {
      UtilThing();
      ~UtilThing();

    public:
      UtilThing(const UtilThing&) = default;
      UtilThing(UtilThing&&) = delete;
      UtilThing& operator=(const UtilThing&) = default;
      UtilThing& operator=(UtilThing&&) = delete;
   };
}