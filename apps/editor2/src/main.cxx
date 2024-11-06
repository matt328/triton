#include "MyThing.hpp"
#include "config.h"
#include "Application.hpp"

#include "Logger2.hpp"
#include "cm/di.hpp"
#include "cm/IWindow.hpp"
#include "Window.hpp"
#include "Properties.hpp"
#include "cm/FrameworkConfig.hpp"

#include "cm/services/Composition.hpp"
#include "cm/services/IService.hpp"

namespace di = boost::di;

#if defined(TRACY_ENABLE)

auto operator new(const std::size_t count) -> void* {
   const auto ptr = malloc(count);
   TracyAllocS(ptr, count, 32);
   return ptr;
}
void operator delete(void* ptr) noexcept {
   TracyFreeS(ptr, 32);
   free(ptr);
}
#endif

// #ifdef WIN32
// int WINAPI WinMain([[maybe_unused]] _In_ HINSTANCE hInstance,
//                    [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
//                    [[maybe_unused]] _In_ LPSTR lpCmdLine,
//                    [[maybe_unused]] _In_ int nShowCmd) {
// #else
auto main() -> int {
   // #endif
   initLogger();

   Log.info("Console is now ready for logging!");

#ifdef _DEBUG
   Log.set_level(spdlog::level::trace);
#else
   Log.set_level(spdlog::level::warn);
#endif

   Log.info("Hello from spdlog");

   static constexpr int width = 1920;
   static constexpr int height = 1080;

#ifdef _DEBUG
   Log.info("Debug Build");
#else
   Log.info("Release Build");
#endif

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

      auto myThing = []() { return std::make_shared<MyThing>(); };

      auto applicationModule = [&windowTitle, &propertiesPath, &myThing] {
         return di::make_injector(di::bind<IService>.to([&]() { return createService(myThing); }),
                                  di::bind<tr::cm::IWindow>.to<ed::Window>().in(di::singleton),
                                  di::bind<glm::ivec2>.to<>(glm::ivec2{width, height}),
                                  di::bind<std::string>.to<>(windowTitle.str()),
                                  di::bind<std::filesystem::path>.to<>(propertiesPath));
      };

      const auto injector = di::make_injector(applicationModule(), tr::cm::engineModule());

      auto app = injector.create<std::shared_ptr<ed::Application>>();

      Log.info("Initialized");

      app->run();

   } catch (const std::exception& e) {
      Log.critical(e.what());
      return -1;
   }

   return 0;
}