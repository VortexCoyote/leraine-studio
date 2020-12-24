#meta
cmake_minimum_required(VERSION 3.0.0)
project($ENV{PROJECT_NAME} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ../binaries)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../binaries)

#build routine
include_directories($ENV{VCPKG_ROOT}/installed/x64-windows/include include)
link_directories($ENV{VCPKG_ROOT}/installed/x64-windows/lib libraries)

file(GLOB_RECURSE SOURCE_FILES "source/*.cpp" "source/*.cxx" "source/*.c")
file(GLOB_RECURSE HEADER_FILES "source/*.h" "source/*.hpp")

find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(ImGui CONFIG REQUIRED)

add_executable($ENV{PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties($ENV{PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ../binaries)

target_link_libraries($ENV{PROJECT_NAME} PRIVATE
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
    zlib
)