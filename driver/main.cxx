#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// This all needs to be here since I don't understand all the preprocessor black
// magic happening in this library
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tiny_gltf.h>

// This lib also does a bunch of janky stuff with the preprocessor. I miss
// static libs.
#define VMA_IMPLEMENTATION
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

#include "Application.hpp"
#include "Log.hpp"

#define TRACY_ENABLED
#include "tracy/Tracy.hpp"

using Core::Log;

void* operator new(std ::size_t count) {
   auto ptr = malloc(count);
   TracyAlloc(ptr, count);
   return ptr;
}
void operator delete(void* ptr) noexcept {
   TracyFree(ptr);
   free(ptr);
}

int main() {
   Log::init();

   try {
      const auto app = Application{};
      app.run();
   } catch (const std::exception& e) { Log::game->error(e.what()); }
}
