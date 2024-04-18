project("testarea" VERSION "0.0.1" LANGUAGES CXX)

set(testarea_SRC
   "src/main.cxx"
)

add_executable(${PROJECT_NAME} ${testarea_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <iostream>
   <chrono>
   <future>
   <iostream>
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
)
