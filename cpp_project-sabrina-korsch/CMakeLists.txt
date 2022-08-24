cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(pixels LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(pixelLib ./include/pixel_matrix.hpp ./lib/pixel_matrix.cpp  ./include/color_reductions.hpp ./lib/color_reductions.cpp ./include/dithering.hpp ./lib/dithering.cpp)

add_executable(pixel_art_gen ./pixel_art_gen.cpp)
target_link_libraries(pixel_art_gen PUBLIC ${OpenCV_LIBS} pixelLib)

install(TARGETS pixel_art_gen DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/demo DESTINATION ${CMAKE_INSTALL_PREFIX}/bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/demo_assets DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)