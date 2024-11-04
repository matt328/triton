project("triton-di")

set(common_SRC
   modules/common/src/Timer.cxx
   modules/gameplay/src/GameplaySystem.cxx
   modules/graphics/src/Renderer.cxx
)

add_library(${PROJECT_NAME} STATIC ${common_SRC})

IF(WIN32)
   target_compile_definitions(${PROJECT_NAME}
      PRIVATE
      -D_CRT_SECURE_NO_WARNINGS
   )
ENDIF(WIN32)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PUBLIC
   <functional>
   <glm/glm.hpp>
   <random>
   <bitset>
   <map>
   <format>
   <filesystem>
   <thread>
   <tracy/Tracy.hpp>
   <tracy/TracyC.h>
   <typeindex>
   <variant>
   PRIVATE
   <ctime>
   <cstdint>
   <unordered_map>
   <vector>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   glm
   EnTT
   Tracy::TracyClient
   util-lib
   PRIVATE
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   modules/common/include
   modules/gameplay/include
   modules/graphics/include
   PRIVATE
   modules/common/include/cm
   modules/common/src
   modules/gameplay/include/gp
   modules/gameplay/src
   modules/graphics/include/gfx
   modules/graphics/src
)
