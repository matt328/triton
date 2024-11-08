include(FetchContent)

set(BOOSTDI_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/boost-di)

FetchContent_Declare(
   boost-di-header
   URL https://raw.githubusercontent.com/boost-ext/di/cpp14/include/boost/di.hpp
   DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_Populate(boost-di-header)

file(MAKE_DIRECTORY ${BOOSTDI_INCLUDE_DIR})
file(RENAME ${boost-di-header_SOURCE_DIR}/di.hpp ${BOOSTDI_INCLUDE_DIR}/di.hpp)

add_library(BoostDI INTERFACE)

target_include_directories(BoostDI
   INTERFACE
   ${CMAKE_BINARY_DIR}/_deps/boost-di
)
