#include "Application.h"
#include "Log.h"
// This all needs to be here since I don't understand all the preprocessor black magic happening in
// this library
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

int main() {
  Log::init();

  const auto app = Application{};

  app.run();

  Log::game->info("hello {}", "world");
}
