#pragma once

class IContext;

class ContextFactory {
 public:
   static auto getContext() -> std::shared_ptr<IContext>;
};
