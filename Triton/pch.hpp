#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <memory>
#include <ranges>
#include <set>

#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <tiny_gltf.h>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_raii.hpp>

#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

#include <GLFW/glfw3.h>

#include <ktx.h>
#include <ktxvulkan.h>