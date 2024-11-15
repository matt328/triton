#include "config.h"
#include "Application.hpp"

#include "tr/ComponentFactory.hpp"
#include "Properties.hpp"
#include "TracyDefines.hpp"

namespace di = boost::di;

// #ifdef WIN32
// int WINAPI WinMain([[maybe_unused]] _In_ HINSTANCE hInstance,
//                    [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
//                    [[maybe_unused]] _In_ LPSTR lpCmdLine,
//                    [[maybe_unused]] _In_ int nShowCmd) {
// #else
auto main() -> int {
   // #endif
   initLogger(spdlog::level::trace, spdlog::level::warn);

   Log.info("Console is now ready for logging!");

   static constexpr int width = 1920;
   static constexpr int height = 1080;

   auto windowTitle = std::stringstream{};
   windowTitle << PROJECT_NAME << " v" << PROJECT_VER;

#ifdef _DEBUG
   windowTitle << " - Debug Build";
#else
   ss << " - Release Build";
#endif

   const auto configDir = std::filesystem::path(sago::getConfigHome()) / "editor";
   const auto propertiesPath = configDir / "editor";

   try {

      const auto frameworkConfig = tr::FrameworkConfig{
          .initialWindowSize = glm::ivec2(width, height),
          .windowTitle = windowTitle.str(),
      };

      auto context = tr::ComponentFactory::getContext(frameworkConfig);

      const auto injector = di::make_injector(di::bind<tr::IContext>.to(context),
                                              di::bind<std::filesystem::path>.to<>(propertiesPath));

      auto application = injector.create<std::shared_ptr<ed::Application>>();

      Log.info("Initialized");

      application->run();

   } catch (const std::exception& e) {
      Log.critical(e.what());
      return -1;
   }

   return 0;
}