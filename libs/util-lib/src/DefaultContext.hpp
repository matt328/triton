#include "IContext.hpp"
#include "IEventBus.hpp"

class DefaultContext : public IContext {
 public:
   explicit DefaultContext(std::shared_ptr<IEventBus> newEventBus);
   void setWindow(std::shared_ptr<IWindow> newWindow) override;

 private:
   std::shared_ptr<IEventBus> eventBus;
   std::shared_ptr<IWindow> window;
};