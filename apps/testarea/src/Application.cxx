#include "Application.hpp"

Application::Application(std::shared_ptr<IWindow> newWindow, std::shared_ptr<IContext> newContext)
    : window{std::move(newWindow)}, context{std::move(newContext)} {
   std::cout << "Constructing Application" << '\n';
   context->setWindow(window);
}