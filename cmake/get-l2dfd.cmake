include(FetchContent)

set(L2DFD_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/l2dfd)

FetchContent_Declare(
  l2dfd-header
  URL https://raw.githubusercontent.com/Limeoats/L2DFileDialog/refs/heads/master/L2DFileDialog/src/L2DFileDialog.h
  DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(l2dfd-header)

file(MAKE_DIRECTORY ${L2DFD_INCLUDE_DIR})
file(RENAME ${l2dfd-header_SOURCE_DIR}/L2DFileDialog.h ${L2DFD_INCLUDE_DIR}/L2DFileDialog.hpp)

add_library(L2DFD INTERFACE)

target_include_directories(L2DFD
  INTERFACE
  ${CMAKE_BINARY_DIR}/_deps/l2dfd
)
