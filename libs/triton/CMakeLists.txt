project("triton")

set(triton_SRC
   "src/Context.cxx"
   "src/GameplayFacade.cxx"
)

add_library(${PROJECT_NAME} STATIC ${triton_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

IF(WIN32)
   target_compile_definitions(${PROJECT_NAME}
      PUBLIC
      WIN32_LEAN_AND_MEAN
      PRIVATE
      -D_CRT_SECURE_NO_WARNINGS
      VK_USE_PLATFORM_WIN32_KHR
   )
ENDIF(WIN32)

IF(APPLE)
   target_compile_definitions(${PROJECT_NAME}
      PRIVATE
      VK_USE_PLATFORM_MACOS_MVK
   )
ENDIF(APPLE)

find_package(Tracy CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   NOMINMAX
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <filesystem>
   <future>
   PUBLIC
   <tracy/Tracy.hpp>
   <tracy/TracyC.h>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   common
   graphics
   Tracy::TracyClient
   PRIVATE
   gameplay
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
   include/tr
)

add_dependencies(${PROJECT_NAME} assets)

add_subdirectory(common)
add_subdirectory(gameplay)
add_subdirectory(graphics)
