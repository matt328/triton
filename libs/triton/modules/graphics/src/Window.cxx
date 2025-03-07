#include "Window.hpp"
#include "tr/Events.hpp"
#include "tr/IGuiAdapter.hpp"
#include "tr/KeyMap.hpp"

#ifdef _WIN32
#include <GLFW/glfw3native.h>
#include <windows.h> // For general Windows APIs
#include <dwmapi.h>  // For DWMWINDOWATTRIBUTE
#endif

namespace tr {

constexpr int MinWidth = 320;
constexpr int MinHeight = 200;

Window::Window(std::shared_ptr<IEventBus> newEventBus,
               std::shared_ptr<tr::IGuiAdapter> newGuiAdapter,
               const glm::ivec2& dimensions,
               const std::string& windowTitle)
    : eventBus{std::move(newEventBus)}, guiAdapter{std::move(newGuiAdapter)} {
  Log.trace("Constructing Window");

  glfwSetErrorCallback(errorCallback);

  if (glfwInit() == GLFW_TRUE) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(dimensions.x, dimensions.y, windowTitle.data(), nullptr, nullptr);

#ifdef _WIN32

    auto hWnd = glfwGetWin32Window(window);
    // Paints the background of the window black
    PAINTSTRUCT ps;
    RECT rc;
    HDC hdc = BeginPaint(hWnd, &ps);
    GetClientRect(hWnd, &rc);
    SetBkColor(hdc, RGB(0, 0, 0));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr);
    EndPaint(hWnd, &ps);
    BOOL value = TRUE;
    HRESULT result;
    result = DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
    if (result != S_OK) {
      Log.warn("Error setting Window Attributes");
    }
    RECT rcClient{};
    GetWindowRect(hWnd, &rcClient);
    // I feel like trash for this but i can't figure out how to make it repaint enough of the
    // window to actually update it and glfw doesn't want to support dark mode yet.
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

    glfwSetWindowSizeLimits(window, MinWidth, MinHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

    if (glfwRawMouseMotionSupported() != 0) {
      glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetWindowIconifyCallback(window, windowIconifiedCallback);
    glfwSetWindowCloseCallback(window, windowCloseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
  } else {
    Log.warn("Error initializing GLFW");
  }
}

auto Window::getNativeWindow() -> void* {
  return this->window;
}

void Window::pollEvents() {
  glfwPollEvents();
}

void Window::setVulkanVersion(std::string_view version) {
  const auto* const currentTitle = glfwGetWindowTitle(window);
  auto newWindowTitle = std::string{currentTitle};
  newWindowTitle += " ";
  newWindowTitle += "Vulkan ";
  newWindowTitle += version.data();

  glfwSetWindowTitle(window, newWindowTitle.c_str());
}

void Window::errorCallback(int code, const char* description) {
  Log.critical("GLFW Error Code: {}, description: {}", code, description);
  throw std::runtime_error("GLFW Error. See log output for details");
}

void Window::windowIconifiedCallback(GLFWwindow* window, const int iconified) {
  auto* const thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
  thisWindow->eventBus->emit(tr::WindowIconified{iconified});
}

void Window::windowCloseCallback(GLFWwindow* window) {
  auto* const thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
  thisWindow->eventBus->emit(tr::WindowClosed{});
}

void Window::keyCallback(GLFWwindow* window,
                         const int key,
                         [[maybe_unused]] int scancode,
                         [[maybe_unused]] const int action,
                         const int mods) {

  auto* const thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

  if (thisWindow->guiAdapter->needsKeyboard()) {
    return;
  }

  // Capture Alt+Enter to toggle fullscreen and 'consume' the keystrokes.
  if (key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT && action == GLFW_RELEASE) {
    thisWindow->toggleFullscreen();
    thisWindow->eventBus->emit(tr::Fullscreen{thisWindow->isFullscreen});
    return;
  }

  // Otherwise, translate and emit the key event
  const auto mappedKey = tr::keyMap.at(key);
  auto buttonState = tr::ButtonState::Pressed;
  if (action == GLFW_RELEASE) {
    buttonState = tr::ButtonState::Released;
  }

  thisWindow->eventBus->emit(tr::KeyEvent{mappedKey, buttonState});
}

void Window::cursorPosCallback(GLFWwindow* window, const double xpos, const double ypos) {
  if (auto* const thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
      thisWindow->isMouseCaptured) {
    thisWindow->eventBus->emit(tr::MouseMoved{xpos, ypos});
  }
}

void Window::mouseButtonCallback(GLFWwindow* window,
                                 const int button,
                                 const int action,
                                 const int mods) {
  auto* const thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (thisWindow->guiAdapter->needsMouse() && !thisWindow->isMouseCaptured) {
    return;
  }
  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
    if (!thisWindow->isMouseCaptured) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      thisWindow->guiAdapter->disableMouse();
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      thisWindow->guiAdapter->enableMouse();
    }
    thisWindow->isMouseCaptured = !thisWindow->isMouseCaptured;
    thisWindow->eventBus->emit(tr::MouseCaptured{thisWindow->isMouseCaptured});
  }
  thisWindow->eventBus->emit(tr::MouseButton{button, action, mods});
}

void Window::toggleFullscreen() {
  if (isFullscreen) {
    glfwSetWindowMonitor(window, nullptr, prevXPos, prevYPos, prevWidth, prevHeight, 0);
    isFullscreen = !isFullscreen;
  } else {
    auto* const currentMonitor = glfwGetPrimaryMonitor();
    const auto* const mode = glfwGetVideoMode(currentMonitor);
    glfwGetWindowPos(window, &prevXPos, &prevYPos);
    glfwGetWindowSize(window, &prevWidth, &prevHeight);
    glfwSetWindowMonitor(window,
                         currentMonitor,
                         0,
                         0,
                         mode->width,
                         mode->height,
                         mode->refreshRate);
    isFullscreen = !isFullscreen;
  }
}

Window::~Window() {
  Log.debug("Destroying window");
  if (window != nullptr) {
    glfwDestroyWindow(window);
    window = nullptr;
  }
}
}
