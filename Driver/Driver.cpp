#include "pch.h"

#include "Instance.h"

#include <volk.h>

#include "Log.h"
#include "Shaders.h"

int main() {
   Log::init();
   glslang_initialize_process();

   if (const auto result = volkInitialize(); result != VK_SUCCESS) {
      throw std::runtime_error("Failed to initialize volk");
   }

   auto instance = std::make_unique<Instance>();

   LOG_LDEBUG("Hello World!");
}
