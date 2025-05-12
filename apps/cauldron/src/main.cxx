#include "api/fx/IFrameworkContext.hpp"
#include "fx/FrameworkFactory.hpp"
#include "fx/IGameLoop.hpp"
#include "api/ext/IGameObjectProxy.hpp"
#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "bk/TaskQueue.hpp"
#include "api/fx/IEventBus.hpp"
#include "api/fx/IGameWorldSystem.hpp"

#include "Application.hpp"

// Following aren't referenced in this file, but need to be here for BoostDI to work
#include "Properties.hpp"
#include "gw/IWidgetService.hpp"
#include "ui/Manager.hpp"
#include "ui/components/Menu.hpp"
#include "data/DataFacade.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/AssetViewer.hpp"
#include "ui/components/EntityEditor.hpp"
#include "ui/components/AssetTool.hpp"
#include "ImGuiAdapter.hpp"
#include "config.h"
#include "fx/ThreadedFrameworkContext.hpp"
#include "api/fx/IEventQueue.hpp"

namespace di = boost::di;

// #ifdef WIN32
// int WINAPI WinMain([[maybe_unused]] _In_ HINSTANCE hInstance,
//                    [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
//                    [[maybe_unused]] _In_ LPSTR lpCmdLine,
//                    [[maybe_unused]] _In_ int nShowCmd) {
// #else
auto main() -> int {
  // #endif
  initLogger(spdlog::level::trace, spdlog::level::trace);

  Log.info("Console is now ready for logging!");

  static constexpr int width = 1920;
  static constexpr int height = 1080;

  auto windowTitle = std::stringstream{};
  windowTitle << PROJECT_NAME << " v" << PROJECT_VER;

#ifdef _DEBUG
  windowTitle << " - Debug Build";
#else
  windowTitle << " - Release Build";
#endif

  const auto configDir = std::filesystem::path(sago::getConfigHome()) / "editor";
  auto propertiesPath = configDir / "editor";

  try {
    const auto guiAdapter = std::make_shared<tr::ImGuiAdapter>();
    const auto frameworkConfig = tr::FrameworkConfig{.initialWindowSize = glm::ivec2(width, height),
                                                     .windowTitle = windowTitle.str()};

    const auto frameworkContext = tr::ThreadedFrameworkContext::create(frameworkConfig, guiAdapter);

    const auto injector = di::make_injector(di::bind<std::filesystem::path>.to<>(propertiesPath),
                                            di::bind<tr::IEventQueue>.to<>([&frameworkContext] {
                                              return frameworkContext->getEventQueue();
                                            }));

    auto app = injector.create<std::shared_ptr<ed::Application>>();

    Log.info("Initialized");

    frameworkContext->runMainLoop();

  } catch (const std::exception& e) {
    Log.critical(e.what());
    return -1;
  }

  return 0;
}
