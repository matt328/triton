#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Application.hpp"
#include "Log.hpp"
// This all needs to be here since I don't understand all the preprocessor black magic happening in
// this library
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

// This lib also does a bunch of janky stuff with the preprocessor. I miss static libs.
#define VMA_IMPLEMENTATION
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

int main() {
   Log::init();

   const auto app = Application{};

   app.run();

   Log::game->info("hello {}", "world");
}
