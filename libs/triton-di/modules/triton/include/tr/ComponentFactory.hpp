#pragma once

namespace tr {
   class IContext;

   class ComponentFactory {
    public:
      static auto getContext() -> std::shared_ptr<IContext>;
   };

}