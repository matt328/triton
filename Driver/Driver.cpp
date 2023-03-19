#include "Application.h"
#include "Log.h"

int main() {
  Log::init();

  const auto app = Application{};

  app.run();

  Log::game->info("hello {}", "world");
}
