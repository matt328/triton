#include "Application.hpp"
#include "Properties.hpp"

#include "ctx/Context.hpp"
#include "ctx/GameplayFacade.hpp"
#include "util/Paths.hpp"

#include "ui/Manager.hpp"
#include "data/DataFacade.hpp"
#include <memory>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#include <windows.h> // For general Windows APIs
#include <dwmapi.h>  // For DWMWINDOWATTRIBUTE
#include <uxtheme.h>
#endif

namespace ed {

   constexpr auto MinHeight = 300;
   constexpr auto MinWidth = 200;
   constexpr auto ZNear = 0.1f;
   constexpr auto ZFar = 1000000.f;
   constexpr auto Fov = 60.f;
   constexpr auto CamStart = glm::vec3{1.f, 1.f, 3.f};

   constexpr auto ImguiEnabled = true;
   constexpr auto GameplayDebugEnabled = true;

   Application::Application(const int width, const int height, const std::string_view& windowTitle)
       : window(nullptr), context(nullptr), running(true) {

      const auto configDir = std::filesystem::path(sago::getConfigHome()) / "editor";
      pr::Properties::getInstance().load(configDir / "config.json");

      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

#ifdef _WIN32

      int imageWidth{}, imageHeight{}, channels{};
      unsigned char* iconData = stbi_load((tr::util::Paths::TEXTURES / "icon.png").string().c_str(),
                                          &imageWidth,
                                          &imageHeight,
                                          &channels,
                                          STBI_rgb_alpha);
      if (!iconData) {
         std::cerr << "Failed to load icon image" << std::endl;
         glfwTerminate();
      }

      GLFWimage icon;
      icon.width = imageWidth;
      icon.height = imageHeight;
      icon.pixels = iconData;
      glfwSetWindowIcon(window.get(), 1, &icon);

      auto hWnd = glfwGetWin32Window(window.get());
      // Paints the background of the window black
      PAINTSTRUCT ps;
      RECT rc;
      HDC hdc = BeginPaint(hWnd, &ps);
      GetClientRect(hWnd, &rc);
      SetBkColor(hdc, RGB(0, 0, 0));
      ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr);
      EndPaint(hWnd, &ps);
      BOOL value = TRUE;
      ::DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
      RECT rcClient{};
      GetWindowRect(hWnd, &rcClient);
      // I feel like trash for this but i can't figure out how to make it repaint enough of the
      // window to actually update it and glfw doesn't want to support dark mode yet.
      // TODO: roll my own cross platform windowing library.
      SetWindowPos(hWnd,
                   nullptr,
                   rcClient.left,
                   rcClient.top,
                   rcClient.right - rcClient.left - 1,
                   rcClient.bottom - rcClient.top - 1,
                   SWP_FRAMECHANGED);
      SetWindowPos(hWnd,
                   nullptr,
                   rcClient.left,
                   rcClient.top,
                   rcClient.right - rcClient.left,
                   rcClient.bottom - rcClient.top,
                   SWP_FRAMECHANGED);
#endif

      glfwSetWindowSizeLimits(window.get(), MinHeight, MinWidth, GLFW_DONT_CARE, GLFW_DONT_CARE);

      if (glfwRawMouseMotionSupported()) {
         glfwSetInputMode(window.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
      }

      glfwSetWindowUserPointer(window.get(), this);
      glfwSetWindowCloseCallback(window.get(), windowCloseCallback);
      glfwSetKeyCallback(window.get(), keyCallback);
      glfwSetCursorPosCallback(window.get(), cursorPosCallback);
      glfwSetMouseButtonCallback(window.get(), mouseButtonCallback);
      glfwSetWindowIconifyCallback(window.get(), windowIconifiedCallback);

      context =
          std::make_unique<tr::ctx::Context>(window.get(), ImguiEnabled, GameplayDebugEnabled);

      dataFacade = std::make_unique<data::DataFacade>();

      manager = std::make_unique<ui::Manager>(context->getGameplayFacade(), *dataFacade);

      manager->addQuitListener<&tr::ctx::Context::hostWindowClosed>(context.get());
      manager->setFullscreenFn([this]() { toggleFullscreen(*this); });
      manager->setWireframeFn([this](bool b) { context->setWireframe(b); });

      auto& facade = context->getGameplayFacade();

      auto camera =
          facade.createCamera(width, height, Fov, ZNear, ZFar, CamStart, "Default Camera");
      facade.setCurrentCamera(camera);
   }

   Application::~Application() {
      glfwTerminate();
   }

   void Application::run() {
      context->start([this]() {
         glfwPollEvents();
         ImGui_ImplVulkan_NewFrame();
         ImGui_ImplGlfw_NewFrame();
         ImGui::NewFrame();

         manager->render();

         ImGui::Render();
      });
   }

   // GLFW Callbacks
   void Application::errorCallback(int code, const char* description) {
      Log::error << "GLFW Error. Code: " << code << ", description: " << description << std::endl;
      throw std::runtime_error("GLFW Error. See log output for details");
   }

   void Application::windowIconifiedCallback(GLFWwindow* window, int iconified) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      // Just stop crashing for now.
      app->context->pause(iconified);
      app->paused = iconified;
   }

   void Application::windowCloseCallback(GLFWwindow* window) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      app->running = false;
      app->context->hostWindowClosed();
   }

   void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
      if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantTextInput) {
         return;
      }
      auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      if (key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT && action == GLFW_RELEASE) {
         toggleFullscreen(*app);
      } else {
         app->context->keyCallback(key, scancode, action, mods);
      }
   }

   void Application::toggleFullscreen(Application& app) {
      auto window = app.window.get();
      if (app.fullscreen) {
         glfwSetWindowMonitor(window,
                              nullptr,
                              app.prevXPos,
                              app.prevYPos,
                              app.prevWidth,
                              app.prevHeight,
                              0);
         app.fullscreen = !app.fullscreen;
      } else {
         const auto currentMonitor = glfwGetPrimaryMonitor();
         const auto mode = glfwGetVideoMode(currentMonitor);
         glfwGetWindowPos(window, &app.prevXPos, &app.prevYPos);
         glfwGetWindowSize(window, &app.prevWidth, &app.prevHeight);
         glfwSetWindowMonitor(window,
                              currentMonitor,
                              0,
                              0,
                              mode->width,
                              mode->height,
                              mode->refreshRate);
         app.fullscreen = !app.fullscreen;
      }
   }

   void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      if (app->mouseCaptured) {
         app->context->cursorPosCallback(xpos, ypos);
      }
   }

   void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      if (ImGui::GetIO().WantCaptureMouse && !app->mouseCaptured) {
         return;
      }
      if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
         if (!app->mouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
         } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
         }
         app->mouseCaptured = !app->mouseCaptured;
         app->context->setMouseState(app->mouseCaptured);
      }
      app->context->mouseButtonCallback(button, action, mods);
   }
}