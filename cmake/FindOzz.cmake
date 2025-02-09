find_path(OZZ_INCLUDE_DIR
  NAMES ozz/animation/runtime/animation.h
  PATHS ${CMAKE_SOURCE_DIR}/vendor/ozz/include
  DOC "Path to the Ozz Animation headers"
)

# Try to find the Ozz library files
find_library(OZZ_ANIMATION_LIBRARY
  NAMES ozz_animation_d
  PATHS ${CMAKE_SOURCE_DIR}/vendor/ozz/lib
  DOC "Path to the Ozz Animation library"
)

find_library(OZZ_BASE_LIBRARY
  NAMES ozz_base_d
  PATHS ${CMAKE_SOURCE_DIR}/vendor/ozz/lib
  DOC "Path to the Ozz Base library"
)

# Check if the library and include directories were found
if(NOT OZZ_INCLUDE_DIR)
  message(FATAL_ERROR "Ozz Animation headers not found.")
endif()

if(NOT OZZ_BASE_LIBRARY)
  message(FATAL_ERROR "Ozz Animation library not found.")
endif()

# Provide information to the user
message(STATUS "Found Ozz Animation library at ${OZZ_ANIMATION_LIBRARY}")
message(STATUS "Found Ozz Base library at ${OZZ_BASE_LIBRARY}")
message(STATUS "Found Ozz Animation headers at ${OZZ_INCLUDE_DIR}")

# Set the variables that will be used by the calling project
set(OZZ_INCLUDE_DIRS ${OZZ_INCLUDE_DIR} CACHE INTERNAL "Ozz Animation include directories")

set(OZZ_LIBRARIES
  ${OZZ_BASE_LIBRARY}
  ${OZZ_ANIMATION_LIBRARY}
)

find_package_handle_standard_args(Ozz DEFAULT_MSG OZZ_LIBRARIES OZZ_INCLUDE_DIRS)
