#include "Application.hpp"
#include "ctx/Context.hpp"
#include "ctx/GameplayFacade.hpp"
#include "util/Paths.hpp"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#include <windows.h> // For general Windows APIs
#include <dwmapi.h>  // For DWMWINDOWATTRIBUTE
#include <uxtheme.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace ed {

   constexpr auto MinHeight = 300;
   constexpr auto MinWidth = 200;
   constexpr auto ZNear = 0.1f;
   constexpr auto ZFar = 1000.f;
   constexpr auto Fov = 60.f;
   constexpr auto CamStart = glm::vec3{1.f, 1.f, 3.f};

   constexpr auto ImguiEnabled = true;

   Application::Application(const int width, const int height, const std::string_view& windowTitle)
       : window(nullptr), context(nullptr), running(true) {
      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

      auto hWnd = glfwGetWin32Window(window.get());

      bool value = true;

      SetWindowTheme(hWnd, L"DarkMode_Explorer", nullptr);
      DwmSetWindowAttribute(hWnd, 19, &value, sizeof(value));
      DwmSetWindowAttribute(hWnd, 20, &value, sizeof(value));

      // Paints the background of the window black
      PAINTSTRUCT ps;
      RECT rc;
      HDC hdc = BeginPaint(hWnd, &ps);
      GetClientRect(hWnd, &rc);
      SetBkColor(hdc, BLACK_BRUSH);
      ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
      EndPaint(hWnd, &ps);

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

      context = std::make_unique<tr::ctx::Context>(window.get(), ImguiEnabled);

      // Editor should provide a way to load these things from a file and call this api
      auto& facade = context->getGameplayFacade();

      facade.createStaticMeshEntity((tr::util::Paths::MODELS / "viking_room.gltf").string(),
                                    (tr::util::Paths::TEXTURES / "viking_room.png").string(),
                                    "Viking Room #1");
      facade.createStaticMeshEntity((tr::util::Paths::MODELS / "viking_room.gltf").string(),
                                    (tr::util::Paths::TEXTURES / "viking_room.png").string(),
                                    "Viking Room #2");
      facade.createStaticMeshEntity((tr::util::Paths::MODELS / "viking_room.gltf").string(),
                                    (tr::util::Paths::TEXTURES / "viking_room.png").string(),
                                    "Viking Room #3");
      facade.createStaticMeshEntity((tr::util::Paths::MODELS / "area.gltf").string(),
                                    (tr::util::Paths::TEXTURES / "grass.png").string(),
                                    "Grass Plane");

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
         ImGui::ShowDemoWindow();

         auto& facade = context->getGameplayFacade();

         renderEntityEditor(facade);

         ImGui::Render();
      });
   }

   void Application::renderEntityEditor(tr::ctx::GameplayFacade& facade) {
      auto& es = facade.getAllEntities();

      if (ImGui::Begin("Entity Editor", &active, ImGuiWindowFlags_MenuBar)) {
         // Left
         ImGui::BeginChild("left pane",
                           ImVec2(150, 0),
                           ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
         for (auto e : es) {
            auto& nameComponent = facade.getEntityName(e);
            if (ImGui::Selectable(nameComponent.name.c_str(),
                                  selectedEntity == static_cast<uint32_t>(e))) {
               selectedEntity = static_cast<uint32_t>(e);
            }
         }
         ImGui::EndChild();
         ImGui::SameLine();

         // Right
         {
            ImGui::BeginGroup();
            ImGui::BeginChild(
                "item view",
                ImVec2(0,
                       -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

            ImGui::Text("Entity ID: %d", selectedEntity);
            ImGui::SeparatorText("Transform");

            auto& transform = facade.getEntityPosition(static_cast<entt::entity>(selectedEntity));
            ImGui::DragFloat3("Position", glm::value_ptr(transform.position), .1f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), .1f, -180.f, 180.f);
            ImGui::SeparatorText("Renderable");

            ImGui::EndChild();
            if (ImGui::Button("Revert")) {}
            ImGui::SameLine();
            if (ImGui::Button("Save")) {}
            ImGui::EndGroup();
         }
      }
      ImGui::End();
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
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      if (key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT && action == GLFW_RELEASE) {
         if (app->fullscreen) {
            glfwSetWindowMonitor(window,
                                 nullptr,
                                 app->prevXPos,
                                 app->prevYPos,
                                 app->prevWidth,
                                 app->prevHeight,
                                 0);
            app->fullscreen = !app->fullscreen;
         } else {
            const auto currentMonitor = glfwGetPrimaryMonitor();
            const auto mode = glfwGetVideoMode(currentMonitor);
            glfwGetWindowPos(window, &app->prevXPos, &app->prevYPos);
            glfwGetWindowSize(window, &app->prevWidth, &app->prevHeight);
            glfwSetWindowMonitor(window,
                                 currentMonitor,
                                 0,
                                 0,
                                 mode->width,
                                 mode->height,
                                 mode->refreshRate);
            app->fullscreen = !app->fullscreen;
         }
      } else {
         app->context->keyCallback(key, scancode, action, mods);
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