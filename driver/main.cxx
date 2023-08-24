#define TRACY_ENABLED
#define TRACY_ON_DEMAND
#include "tracy/Tracy.hpp"

// void* operator new(size_t size) {
//    void* ptr = malloc(size);

//    TracySecureAlloc(ptr, size);
//    return ptr;
// }

// void* operator new[](size_t size) {
//    void* ptr = malloc(size);
//    TracySecureAlloc(ptr, size);
//    return ptr;
// }

// void operator delete(void* ptr) noexcept {
//    if (ptr) {
//       TracySecureFree(ptr);
//       free(ptr);
//    }
// }

// void operator delete[](void* ptr) noexcept {
//    if (ptr) {
//       TracySecureFree(ptr);
//       free(ptr);
//    }
// }

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

using Core::Log;

int main() {
   Log::init();

   try {
      const auto app = Application{};
      app.run();
   } catch (const std::exception& e) { Log::game->error(e.what()); }
}
