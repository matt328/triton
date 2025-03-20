#include "fx/IFrameworkContext.hpp"
#include "fx/FrameworkFactory.hpp"
#include "fx/IGameLoop.hpp"
#include "fx/IGuiCallbackRegistrar.hpp"
#include "fx/ITaskQueue.hpp"
#include "fx/IEventBus.hpp"
#include "fx/IEntityServiceProvider.hpp"
#include "fx/IGameplaySystem.hpp"

#include "Application.hpp"
#include "Properties.hpp"

#include "config.h"

namespace di = boost::di;

// #ifdef WIN32
// int WINAPI WinMain([[maybe_unused]] _In_ HINSTANCE hInstance,
//                    [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
//                    [[maybe_unused]] _In_ LPSTR lpCmdLine,
//                    [[maybe_unused]] _In_ int nShowCmd) {
// #else
auto main() -> int {
  // #endif
  initLogger(spdlog::level::err, spdlog::level::err);

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
    const auto frameworkConfig = tr::FrameworkConfig{
        .initialWindowSize = glm::ivec2(width, height),
        .windowTitle = windowTitle.str(),
    };

    auto frameworkContext = tr::createFrameworkContext(frameworkConfig);

    const auto injector = di::make_injector(
        di::bind<std::filesystem::path>.to<>(propertiesPath),
        di::bind<tr::IGameLoop>.to([&frameworkContext] { return frameworkContext->getGameLoop(); }),
        di::bind<tr::IGuiCallbackRegistrar>.to(
            [&frameworkContext] { return frameworkContext->getGuiCallbackRegistrar(); }),
        di::bind<tr::ITaskQueue>.to(
            [&frameworkContext] { return frameworkContext->getTaskQueue(); }),
        di::bind<tr::IEventBus>.to([&frameworkContext] { return frameworkContext->getEventBus(); }),
        di::bind<tr::IEntityServiceProvider>.to(
            [&frameworkContext] { return frameworkContext->getEntityServiceProvider(); }),
        di::bind<tr::IGameplaySystem>.to(
            [&frameworkContext] { return frameworkContext->getGameplaySystem(); }));

    auto application = injector.create<std::shared_ptr<ed::Application>>();

    Log.info("Initialized");

    application->run();

  } catch (const std::exception& e) {
    Log.critical(e.what());
    return -1;
  }

  return 0;
}
