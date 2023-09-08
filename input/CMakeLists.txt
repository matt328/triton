set(name input)

set(${name}_SRC
   src/GlfwKeyMap.cxx
   src/ActionManager.cxx
)

set(${name}_INCLUDE "include")

add_library(${name} STATIC ${input_SRC})

target_precompile_headers(${name}
   PRIVATE
   <GLFW/glfw3.h>
   <map>
   <utility>
   <variant>
   <cassert>
   <functional>
   <unordered_map>
   <variant>
)

find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(${name}
   PRIVATE
   glfw
)

target_include_directories(${name}
   PUBLIC
   ${input_INCLUDE}
   PRIVATE
   src
)
