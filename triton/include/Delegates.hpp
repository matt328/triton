#pragma once

#include "Action.hpp"

namespace Triton::Actions {

   class Delegate {
    public:
      Delegate() = default;

      size_t addDelegate(std::function<void(Action)> fn) {
         const auto id = callbacks.size();
         callbacks.push_back(fn);
         return id;
      }

      void removeDelegate(size_t id) {
         assert(id < callbacks.size() - 1);
         callbacks.erase(callbacks.begin() + id);
      }

      void operator()(Action value) const {
         for (const auto& c : callbacks) {
            c(value);
         }
      }

    private:
      std::vector<std::function<void(Action)>> callbacks;
   };
}