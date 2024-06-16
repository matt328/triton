project("fbxtool" VERSION "0.0.1" LANGUAGES CXX)

set(fbxtool_SRC
   "src/main.cxx"
)

add_executable(${PROJECT_NAME} ${fbxtool_SRC})

find_package(glm CONFIG REQUIRED)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   triton
   ufbx
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
)
