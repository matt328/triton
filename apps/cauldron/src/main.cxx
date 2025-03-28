#include "config.h"
#include "Application.hpp"

#include "tr/ComponentFactory.hpp"
#include "Properties.hpp"
#include "cm/TaskQueue.hpp"
#include "TracyDefines.hpp"
#include "tr/IEventBus.hpp"
#include <entt/entity/fwd.hpp>

#include "ui/components/AssetTool.hpp"

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
    const auto frameworkConfig = tr::FrameworkConfig{
        .initialWindowSize = glm::ivec2(width, height),
        .windowTitle = windowTitle.str(),
    };

    auto context = tr::ComponentFactory::getContext(frameworkConfig);
    auto gameplaySystem = context->getGameplaySystem();
    auto guiSystem = context->getGuiSystem();
    auto eventSystem = context->getEventSystem();
    auto entityService = gameplaySystem->getEntityService();
    auto taskQueue = context->getTaskQueue();

    const auto injector = di::make_injector(di::bind<tr::IContext>.to(context),
                                            di::bind<tr::IGuiSystem>.to(guiSystem),
                                            di::bind<tr::IEventBus>.to(eventSystem),
                                            di::bind<tr::IGameplaySystem>.to<>(gameplaySystem),
                                            di::bind<tr::TaskQueue>.to<>(taskQueue),
                                            di::bind<std::filesystem::path>.to<>(propertiesPath),
                                            di::bind<tr::EntityService>.to(entityService));

    auto application = injector.create<std::shared_ptr<ed::Application>>();

    Log.info("Initialized");

    application->run();

  } catch (const std::exception& e) {
    Log.critical(e.what());
    return -1;
  }

  return 0;
}
