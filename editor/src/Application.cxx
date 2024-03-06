#include "Application.hpp"
#include "ctx/Context.hpp"
#include "ctx/GameplayFacade.hpp"
#include "util/Paths.hpp"
#include "ImGuiStyle.hpp"
#include "ImFileBrowser.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include "ProjectFile.hpp"
#include "gp/ecs/component/Transform.hpp"

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
   constexpr auto ZFar = 1000.f;
   constexpr auto Fov = 60.f;
   constexpr auto CamStart = glm::vec3{1.f, 1.f, 3.f};

   constexpr auto ImguiEnabled = true;
   constexpr auto GameplayDebugEnabled = true;

   Application::Application(const int width, const int height, const std::string_view& windowTitle)
       : window(nullptr), context(nullptr), running(true) {
      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

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

#ifdef _WIN32
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

      ImGuiEx::setupImGuiStyle();

      // Editor should provide a way to load these things from a file and call this api
      auto& facade = context->getGameplayFacade();

      // facade.createStaticMeshEntity((tr::util::Paths::MODELS / "viking_room.gltf").string(),
      //                               (tr::util::Paths::TEXTURES / "viking_room.png").string(),
      //                               "Viking Room #1");

      // facade.createStaticMeshEntity((tr::util::Paths::MODELS / "viking_room.gltf").string(),
      //                               (tr::util::Paths::TEXTURES / "viking_room.png").string(),
      //                               "Viking Room #2");

      // facade.createStaticMeshEntity((tr::util::Paths::MODELS / "viking_room.gltf").string(),
      //                               (tr::util::Paths::TEXTURES / "viking_room.png").string(),
      //                               "Viking Room #3");

      // facade.createStaticMeshEntity((tr::util::Paths::MODELS / "area.gltf").string(),
      //                               (tr::util::Paths::TEXTURES / "grass.png").string(),
      //                               "Grass Plane");

      auto camera =
          facade.createCamera(width, height, Fov, ZNear, ZFar, CamStart, "Default Camera");
      facade.setCurrentCamera(camera);

      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

      // (optional) set browser properties
      fileDialog.SetTitle("title");
      fileDialog.SetTypeFilters({".hpp", ".cxx"});
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

         renderDockSpace();

         renderEntityEditor(facade);

         ImGui::Render();
      });
   }

   void Application::renderDockSpace() {
      static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
      ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

      if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
         window_flags |= ImGuiWindowFlags_NoBackground;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace", nullptr, window_flags);
      ImGui::PopStyleVar();
      ImGui::PopStyleVar(2);

      // DockSpace
      ImGuiIO& io = ImGui::GetIO();
      if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
         ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
         ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

         static auto first_time = true;
         if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id,
                                                            ImGuiDir_Left,
                                                            0.2f,
                                                            nullptr,
                                                            &dockspace_id);
            ImGui::DockBuilderDockWindow("Entity Editor", dock_id_left);
            ImGui::DockBuilderFinish(dockspace_id);
         }
      }
      ImGui::End(); // Dockspace

      if (ImGui::BeginMainMenuBar()) {
         if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
               context->getGameplayFacade().clear();
               io::readProjectFile(std::string_view{"some_file.json"},
                                   context->getGameplayFacade());
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
               io::writeProjectFile(std::string_view{"some_file.json"},
                                    context->getGameplayFacade());
            }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
               context->hostWindowClosed();
            }
            ImGui::EndMenu();
         }
         if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {} // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
         }
         ImGui::EndMainMenuBar();
      }
   }

   void Application::renderEntityEditor(tr::ctx::GameplayFacade& facade) {
      auto& es = facade.getAllEntities();

      if (ImGui::Begin("Entity Editor")) {
         // Left
         ImGui::BeginChild("left pane",
                           ImVec2(150, 0),
                           ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
         for (auto e : es) {
            auto& infoComponent = facade.getEditorInfo(e);
            if (ImGui::Selectable(infoComponent.name.c_str(),
                                  static_cast<uint32_t>(e) == selectedEntity)) {
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

            if (selectedEntity.has_value()) {
               ImGui::Text("Entity ID: %d", selectedEntity.value());
            } else {
               ImGui::Text("No Entity Selected");
            }

            if (selectedEntity.has_value()) {
               const auto maybeTransform = facade.getComponent<tr::gp::ecs::Transform>(
                   static_cast<entt::entity>(selectedEntity.value()));
               if (maybeTransform.has_value()) {
                  auto& transform = maybeTransform.value().get();
                  ImGui::SeparatorText("Transform");
                  ImGui::DragFloat3("Position", glm::value_ptr(transform.position), .1f);
                  ImGui::DragFloat3("Rotation",
                                    glm::value_ptr(transform.rotation),
                                    .1f,
                                    -180.f,
                                    180.f);
                  ImGui::SeparatorText("Renderable");
               }
            }
            ImGui::EndChild();

            if (ImGui::Button("New...")) {
               fileDialog.Open();
            }
            ImGui::SameLine();
            if (ImGui::Button("Save")) {}

            ImGui::EndGroup();
         }
      }
      ImGui::End();

      fileDialog.Display();

      if (fileDialog.HasSelected()) {
         Log::info << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
         fileDialog.ClearSelected();
      }
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