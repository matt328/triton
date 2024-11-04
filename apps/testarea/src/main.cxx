#include "di.hpp"

#include "View.hpp"

namespace di = boost::di;

struct Renderer {
   explicit Renderer(int device) : device(device) {
   }
   int device;
};

class Model {};

class Controller {
 public:
   Controller(Model& model, IView& view) : view{view} {
   }
   void sayHello() {
      view.update();
      std::cout << "Hello from Controller " << '\n';
   }

 private:
   IView& view;
};

class User {};

class App {
 public:
   App(Controller& controller, User& user) : controller{controller} {
   }

   void doSomething() {
      controller.sayHello();
   }

 private:
   Controller& controller;
};

auto main() -> int {
   auto injector =
       di::make_injector(di::bind<IView>.to<GuiView>(), di::bind<std::string>.to("Bound String"));
   auto app = injector.create<App>();

   app.doSomething();

   return 0;
}
