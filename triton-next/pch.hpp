#pragma once

// Precompiled headers go here

#include "core/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <GLFW/glfw3.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <sstream>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
#include <vulkan/vulkan_raii.hpp>