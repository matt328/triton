#include "ctx/Context.hpp"
#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"
#include "ctx/GameplayFacade.hpp"

namespace tr::ctx {
   constexpr auto SleepMillis = 100;
   static constexpr int TARGET_FPS = 60;
   static constexpr int MAX_UPDATES = 4;

   Context::Context(void* nativeWindow, bool guiEnabled, bool debugEnabled)
       : timer{TARGET_FPS, MAX_UPDATES} {
      gameplaySystem = std::make_unique<gp::GameplaySystem>();

      renderContext =
          std::make_unique<gfx::RenderContext>(static_cast<GLFWwindow*>(nativeWindow), guiEnabled);

      renderContext->addResizeListener<&gp::GameplaySystem::resize>(gameplaySystem.get());

      gameplaySystem->addRenderObjectListener<&gfx::RenderContext::enqueueRenderObject>(
          renderContext.get());

      gameplaySystem->addCameraDataListener<&gfx::RenderContext::setCurrentCameraData>(
          renderContext.get());

      gameplayFacade =
          std::make_unique<GameplayFacade>(*gameplaySystem, *renderContext, debugEnabled);
   }

   // Since a default destructor doesn't have access to the complete definition of the forward
   // declared classes wrapped in smart pointers, it cannot call their destructors. In short, this
   // has to be here so all the member smart pointers will be able to be cleaned up.
   Context::~Context() {
   }

   void Context::start(std::function<void()> pollFn) {
      running = true;
      while (running) {
         pollFn();
         if (this->paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SleepMillis));
            continue;
         }
         timer.tick([&]() { gameplaySystem->fixedUpdate(timer); });
         gameplaySystem->update();

         renderContext->render();
         FrameMark;
      }
      renderContext->waitIdle();
   }

   void Context::pause(bool paused) {
      this->paused = paused;
   }

   void Context::keyCallback(int key, int scancode, int action, int mods) {
      gameplaySystem->keyCallback(key, scancode, action, mods);
   }

   void Context::cursorPosCallback(double xpos, double ypos) {
      gameplaySystem->cursorPosCallback(xpos, ypos);
   }

   void Context::mouseButtonCallback(int button, int action, int mods) {
      gameplaySystem->mouseButtonCallback(button, action, mods);
   }

   void Context::setMouseState(bool captured) {
      gameplaySystem->setMouseState(captured);
   }

   void Context::hostWindowClosed() {
      running = false;
   }
}