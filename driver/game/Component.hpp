#pragma once

namespace Game {
   class Component {
    public:
      Component() = default;
      Component(const Component&) = default;
      Component(Component&&) = delete;
      Component& operator=(const Component&) = default;
      Component& operator=(Component&&) = delete;
      virtual ~Component() = default;

      virtual void update() = 0;
   };
}
