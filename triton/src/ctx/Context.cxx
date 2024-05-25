#include "ctx/Context.hpp"
#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"
#include "ctx/GameplayFacade.hpp"

#include "gp/ska/Animations.hpp"

namespace tr::ctx {
   constexpr auto SleepMillis = 100;
   static constexpr int TARGET_FPS = 60;
   static constexpr int MAX_UPDATES = 4;

   Context::Context(void* nativeWindow, bool guiEnabled, bool debugEnabled)
       : timer{TARGET_FPS, MAX_UPDATES} {

      animations = std::make_unique<gp::ska::Animations>();

      renderContext =
          std::make_unique<gfx::RenderContext>(static_cast<GLFWwindow*>(nativeWindow), guiEnabled);

      auto& animationFactory = renderContext->getResourceManager().getAnimationFactory();

      gameplaySystem = std::make_unique<gp::GameplaySystem>(animationFactory);

      renderContext->addResizeListener<&gp::GameplaySystem::resize>(gameplaySystem.get());

      // Have this take in a reference to the resource manager
      gameplaySystem->addRenderDataListener<&gfx::tx::ResourceManager::setRenderData>(
          &renderContext->getResourceManager());

      gameplayFacade =
          std::make_unique<GameplayFacade>(*gameplaySystem, *renderContext, debugEnabled);
   }

   // Since a default destructor doesn't have access to the complete definition of the forward
   // declared classes wrapped in smart pointers, it cannot call their destructors. In short, this
   // has to be here so all the member smart pointers will be able to be cleaned up.
   Context::~Context() {
   }

   void Context::setWireframe(bool wireframeEnabled) {
      renderContext->setDebugRendering(wireframeEnabled);
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
         gameplaySystem->update(timer.getBlendingFactor());

         animations->update(.016f);

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