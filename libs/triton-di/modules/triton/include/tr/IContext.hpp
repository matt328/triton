#pragma once

namespace tr {

   class IContext {
    public:
      IContext() = default;
      virtual ~IContext() = default;

      IContext(const IContext&) = default;
      IContext(IContext&&) = delete;
      auto operator=(const IContext&) -> IContext& = default;
      auto operator=(IContext&&) -> IContext& = delete;

      virtual void run() = 0;
   };
}