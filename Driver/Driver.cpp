#include "Instance.h"
#include "Log.h"
#include "RenderDevice.h"

int main() {
   Log::init();

   auto instance = std::make_unique<Instance>(true, 1366, 768);

   auto renderDevice = std::make_unique<RenderDevice>(instance);

   Log::game->info("hello {}", "world");
   Log::game->error("Error happened, but not really");
}
