#include "tr/Context.hpp"

#include "cm/Timer.hpp"

#include "tr/GameplayFacade.hpp"
#include "gp/GameplaySystem.hpp"

#include "gfx/RenderContext.hpp"

namespace tr::ctx {
   constexpr auto SleepMillis = 100;
   static constexpr int TARGET_FPS = 60;
   static constexpr int MAX_UPDATES = 4;

   class Context::Context::Impl {
    public:
      Impl(void* nativeWindow, bool guiEnabled, bool debugEnabled)
          : timer{TARGET_FPS, MAX_UPDATES} {
         renderContext =
             std::make_unique<gfx::RenderContext>(static_cast<GLFWwindow*>(nativeWindow),
                                                  guiEnabled);

         gameplaySystem = std::make_unique<gp::GameplaySystem>();

         renderContext->setResizeListener(
             [this](std::pair<uint32_t, uint32_t> size) { this->gameplaySystem->resize(size); });

         gameplaySystem->setRenderDataFn(
             [this](cm::RenderData& renderData) { renderContext->setRenderData(renderData); });

         // Have the facade also take a reference to the resourceManager
         gameplayFacade =
             std::make_unique<GameplayFacade>(*gameplaySystem, *renderContext, debugEnabled);
      }

      [[nodiscard]] GameplayFacade& getGameplayFacade() const {
         return *gameplayFacade;
      }

      void setWireframe(bool wireframeEnabled) {
         renderContext->setDebugRendering(wireframeEnabled);
      }

      void start(std::function<void()> pollFn) {
         running = true;
         while (running) {
            pollFn();
            if (this->paused) {
               std::this_thread::sleep_for(std::chrono::milliseconds(SleepMillis));
               continue;
            }
            timer.tick([&]() { gameplaySystem->fixedUpdate(timer); });
            gameplaySystem->update(timer.getBlendingFactor());

            renderContext->render();
            FrameMark;
         }
         renderContext->waitIdle();
      }

      void pause(bool paused) {
         this->paused = paused;
      }

      void keyCallback(cm::Key key, cm::ButtonState buttonState) {
         gameplaySystem->keyCallback(key, buttonState);
      }

      void cursorPosCallback(double xpos, double ypos) {
         gameplaySystem->cursorPosCallback(xpos, ypos);
      }

      void mouseButtonCallback(int button, int action, int mods) {
         gameplaySystem->mouseButtonCallback(button, action, mods);
      }

      void setMouseState(bool captured) {
         gameplaySystem->setMouseState(captured);
      }

      void hostWindowClosed() {
         running = false;
      }

    private:
      bool running{}, paused{};
      tr::cm::Timer timer;

      std::unique_ptr<GameplayFacade> gameplayFacade;

      std::unique_ptr<tr::gp::GameplaySystem> gameplaySystem;
      std::unique_ptr<tr::gfx::RenderContext> renderContext;
   };

   Context::Context(void* nativeWindow, bool guiEnabled, bool debugEnabled)
       : impl(std::make_unique<Impl>(nativeWindow, guiEnabled, debugEnabled)) {
   }

   // Since a default destructor doesn't have access to the complete definition of the forward
   // declared classes wrapped in smart pointers, it cannot call their destructors. In short, this
   // has to be here so all the member smart pointers will be able to be cleaned up.
   Context::~Context() {
   }

   void Context::setWireframe(bool wireframeEnabled) {
      impl->setWireframe(wireframeEnabled);
   }

   [[nodiscard]] GameplayFacade& Context::getGameplayFacade() const {
      return impl->getGameplayFacade();
   }

   void Context::start(std::function<void()> pollFn) {
      impl->start(pollFn);
   }

   void Context::pause(bool paused) {
      impl->pause(paused);
   }

   void Context::keyCallback(cm::Key key, cm::ButtonState buttonState) {
      impl->keyCallback(key, buttonState);
   }

   void Context::cursorPosCallback(double xpos, double ypos) {
      impl->cursorPosCallback(xpos, ypos);
   }

   void Context::mouseButtonCallback(int button, int action, int mods) {
      impl->mouseButtonCallback(button, action, mods);
   }

   void Context::setMouseState(bool captured) {
      impl->setMouseState(captured);
   }

   void Context::hostWindowClosed() {
      impl->hostWindowClosed();
   }
}