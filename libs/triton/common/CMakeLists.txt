project("common")

set(common_SRC
   "src/Thing.cxx"
)

add_library(${PROJECT_NAME} STATIC ${common_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

IF(WIN32)
   target_compile_definitions(${PROJECT_NAME}
      PRIVATE
      -D_CRT_SECURE_NO_WARNINGS
   )
ENDIF(WIN32)

find_package(glm CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PUBLIC
   <chrono>
   <cstdint>
   <functional>
   <glm/glm.hpp>
   <cm/Logger.hpp>
   <random>
   <tracy/Tracy.hpp>
   <tracy/TracyC.h>
   <unordered_map>
   <vector>
   PRIVATE
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   glm
   Tracy::TracyClient
   PRIVATE
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   include/cm
   src
)
