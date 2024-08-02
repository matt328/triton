#include "tr/Context.hpp"

#include "cm/Timer.hpp"

#include "tr/GameplayFacade.hpp"
#include "gp/GameplaySystem.hpp"

#include "gfx/RenderContext.hpp"

namespace tr::ctx {
   constexpr auto SleepMillis = 100;
   static constexpr int TARGET_FPS = 60;
   static constexpr int MAX_UPDATES = 4;

   class Context::Impl {
    public:
      Impl(void* nativeWindow, bool guiEnabled) : timer{TARGET_FPS, MAX_UPDATES} {
         renderContext =
             std::make_unique<gfx::RenderContext>(static_cast<GLFWwindow*>(nativeWindow),
                                                  guiEnabled);

         gameplaySystem = std::make_unique<gp::GameplaySystem>();

         renderContext->setResizeListener([this](const std::pair<uint32_t, uint32_t> size) {
            this->gameplaySystem->resize(size);
         });

         gameplaySystem->setRenderDataFn([this](const cm::gpu::RenderData& renderData) {
            renderContext->setRenderData(renderData);
         });

         // Have the facade also take a reference to the resourceManager
         gameplayFacade = std::make_unique<GameplayFacade>(*gameplaySystem, *renderContext);
      }

      [[nodiscard]] GameplayFacade& getGameplayFacade() const {
         return *gameplayFacade;
      }

      void setWireframe(const bool wireframeEnabled) const {
         renderContext->setDebugRendering(wireframeEnabled);
      }

      void start(const std::function<void()>& pollFn) {
         while (running) {

            {
               ZoneNamedN(poll, "Poll", true);
               pollFn();
            }
            if (this->paused) {
               std::this_thread::sleep_for(std::chrono::milliseconds(SleepMillis));
               continue;
            }

            {
               ZoneNamedN(fixedUpdate, "Gameplay FixedUpdate", true);
               timer.tick([&] { gameplaySystem->fixedUpdate(timer); });
            }

            {
               ZoneNamedN(z, "Gameplay Update", true);
               gameplaySystem->update(timer.getBlendingFactor());
            }

            {
               ZoneNamedN(z, "RenderContext Render", true);
               renderContext->render();
            }
            FrameMark;
         }
         renderContext->waitIdle();
      }

      void pause(const bool paused) {
         this->paused = paused;
      }

      void keyCallback(const cm::Key key, const cm::ButtonState buttonState) const {
         gameplaySystem->keyCallback(key, buttonState);
      }

      void cursorPosCallback(const double xpos, const double ypos) const {
         gameplaySystem->cursorPosCallback(xpos, ypos);
      }

      void mouseButtonCallback(const int button, const int action, const int mods) const {
         gameplaySystem->mouseButtonCallback(button, action, mods);
      }

      void setMouseState(const bool captured) const {
         gameplaySystem->setMouseState(captured);
      }

      void hostWindowClosed() {
         running = false;
      }

    private:
      bool running{true}, paused{};
      cm::Timer timer;

      std::unique_ptr<GameplayFacade> gameplayFacade;

      std::unique_ptr<gp::GameplaySystem> gameplaySystem;
      std::unique_ptr<gfx::RenderContext> renderContext;
   };

   Context::Context(void* nativeWindow, bool guiEnabled)
       : impl(std::make_unique<Impl>(nativeWindow, guiEnabled)) {
   }

   // Since a default destructor doesn't have access to the complete definition of the forward
   // declared classes wrapped in smart pointers, it cannot call their destructors. In short, this
   // has to be here so all the member smart pointers will be able to be cleaned up.
   Context::~Context() { // NOLINT(*-use-equals-default)
   }

   void Context::setWireframe(const bool wireframeEnabled) const {
      impl->setWireframe(wireframeEnabled);
   }

   [[nodiscard]] GameplayFacade& Context::getGameplayFacade() const {
      return impl->getGameplayFacade();
   }

   void Context::start(const std::function<void()>& pollFn) const {
      impl->start(pollFn);
   }

   void Context::pause(const bool paused) const {
      impl->pause(paused);
   }

   void Context::keyCallback(const cm::Key key, const cm::ButtonState buttonState) const {
      impl->keyCallback(key, buttonState);
   }

   void Context::cursorPosCallback(const double xpos, const double ypos) const {
      impl->cursorPosCallback(xpos, ypos);
   }

   void Context::mouseButtonCallback(const int button, const int action, const int mods) const {
      impl->mouseButtonCallback(button, action, mods);
   }

   void Context::setMouseState(const bool captured) const {
      impl->setMouseState(captured);
   }

   void Context::hostWindowClosed() const {
      impl->hostWindowClosed();
   }
}