#pragma once

// Precompiled headers go here

#include "core/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <map>
#include <optional>
#include <set>
#include <stb_image.h>
#include <string>
#include <string_view>
#include <sstream>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
#include <vulkan/vulkan_raii.hpp>