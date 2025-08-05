cmake_minimum_required(VERSION 3.16)

# Allow duplicate custom targets (e.g., uninstall from subprojects like GLFW)
cmake_policy(SET CMP0002 OLD)
# Use <Package>_ROOT variables for find_package
cmake_policy(SET CMP0074 NEW)

project(dkViewer LANGUAGES CXX)

# 1) C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2) Prevent in-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "Please create a separate build directory.")
endif()

# 3) FetchContent module
include(FetchContent)

# ----------------------------------------------------------------------------
# 4) GLFW via FetchContent + add_subdirectory
# ----------------------------------------------------------------------------
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_Populate(glfw)
add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)

# ----------------------------------------------------------------------------
# 5) GLAD via FetchContent + add_subdirectory
# ----------------------------------------------------------------------------
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.34
)
FetchContent_Populate(glad)
add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR} EXCLUDE_FROM_ALL)

# ----------------------------------------------------------------------------
# 6) ImGui (docking + std::string)
# ----------------------------------------------------------------------------
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        docking
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
  )
  target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
  )
  target_link_libraries(imgui PUBLIC
    glfw
    glad
  )
  target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
  target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
endif()

# ----------------------------------------------------------------------------
# 7) Assimp via FetchContent
# ----------------------------------------------------------------------------
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v5.2.5
)
FetchContent_MakeAvailable(assimp)

# ----------------------------------------------------------------------------
# 8) Eigen (system package)
# ----------------------------------------------------------------------------
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# ----------------------------------------------------------------------------
# 9) System OpenGL
# ----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)

# ----------------------------------------------------------------------------
# 10) Executable target
# ----------------------------------------------------------------------------
add_executable(dkViewer
  src/main.cpp
  src/Scene.cpp
  src/Mesh.cpp
  src/Camera.cpp
)

# ------------------------------------------------------------------------
# 10.a) Treat include/ headers as part of the project
# ------------------------------------------------------------------------
# Collect all public headers so VS shows them under "Header Files"
file(GLOB_RECURSE DKVIEWER_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)
# Add headers to the target (won't compile them)
target_sources(dkViewer PRIVATE ${DKVIEWER_HEADERS})
# Organize the headers in Visual Studio
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${DKVIEWER_HEADERS})

# Expose assets directory to code via compile definition
set(ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")
target_compile_definitions(dkViewer PRIVATE ASSETS_DIR="${ASSETS_DIR}")

# Include directories for our code
target_include_directories(dkViewer PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(dkViewer PRIVATE
  glfw
  glad
  OpenGL::GL
  imgui
  assimp::assimp
  Eigen3::Eigen
)

# ----------------------------------------------------------------------------
# 11) Copy assets
# ----------------------------------------------------------------------------
add_custom_target(copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets
          ${CMAKE_BINARY_DIR}/assets
)
add_dependencies(dkViewer copy_assets)