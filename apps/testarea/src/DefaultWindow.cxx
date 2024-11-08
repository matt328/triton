#include "DefaultWindow.hpp"

DefaultWindow::DefaultWindow() {
   std::cout << "Constructing Window" << '\n';
}

void DefaultWindow::doWindowThing() {
}

void DefaultWindow::registerEventBus(std::shared_ptr<IEventBus> newEventBus) {
   std::cout << "DefautlWindow::registerEventBus" << '\n';
   eventBus = std::move(newEventBus);
}