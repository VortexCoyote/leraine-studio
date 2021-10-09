#meta
cmake_minimum_required(VERSION 3.0.0)
set(PROJECT_NAME leraine-studio)
project(${PROJECT_NAME} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

set(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/binaries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${WORKING_DIRECTORY}")

set(GCC_COVERAGE_COMPILE_FLAGS "lm")

#build routine
include_directories($ENV{VCPKG_ROOT}/installed/x64-linux/include include)
link_directories($ENV{VCPKG_ROOT}/installed/x64-linux/lib libraries)

file(GLOB_RECURSE SOURCE_FILES "source/*.cpp" "source/*.cxx" "source/*.c")
file(GLOB_RECURSE HEADER_FILES "source/*.h" "source/*.hpp")

find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(ImGui CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Copy asset files to binaries/
file(GLOB_RECURSE ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
foreach(ONE_ASSET ${ASSETS})
  file(RELATIVE_PATH RELATIVE_ASSET "${CMAKE_CURRENT_SOURCE_DIR}/assets/" "${ONE_ASSET}")
  set(ASSET_DESTINATION "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${RELATIVE_ASSET}")
  get_filename_component(ASSET_DESTINATION_DIR "${ASSET_DESTINATION}" DIRECTORY)
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${ASSET_DESTINATION_DIR})
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${ONE_ASSET}
    ${ASSET_DESTINATION_DIR})
endforeach()

target_link_libraries(${PROJECT_NAME} PRIVATE
    ImGui-SFML::ImGui-SFML
    FLAC
    OpenAL
    OpenGL
    Vorbis
    sfml-graphics
    sfml-window
    sfml-audio
    sfml-system
    bass
    bass_fx
    ZLIB::ZLIB
    yaml-cpp
)