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
FetchContent_MakeAvailable(glfw)
#add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)

# ----------------------------------------------------------------------------
# 5) GLAD via FetchContent + add_subdirectory
# ----------------------------------------------------------------------------
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.34
)
FetchContent_MakeAvailable(glad)
#add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR} EXCLUDE_FROM_ALL)

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
# 8) Eigen (find or fetch if missing)
# ----------------------------------------------------------------------------
# Try system/package-manager first
# ----------------------------------------------------------------------------
# Eigen (find headers or fetch if missing) -> always provide Eigen3::Eigen
# ----------------------------------------------------------------------------
# Try to locate installed headers (vcpkg/system installs usually put them under eigen3/)
find_path(EIGEN3_INCLUDE_DIR Eigen/Dense
          PATH_SUFFIXES eigen3 eigen
          DOC "Path to Eigen3 headers")

if (EIGEN3_INCLUDE_DIR)
  message(STATUS "Found Eigen headers at: ${EIGEN3_INCLUDE_DIR}")
  add_library(eigen3_headers INTERFACE)
  target_include_directories(eigen3_headers INTERFACE "${EIGEN3_INCLUDE_DIR}")
else()
  message(STATUS "Eigen3 not found — fetching 3.4.0 via FetchContent")
  # FetchContent is already included earlier; if not, add: include(FetchContent)
  FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        3.4.0
  )
  FetchContent_Populate(eigen)
  add_library(eigen3_headers INTERFACE)
  target_include_directories(eigen3_headers INTERFACE "${eigen_SOURCE_DIR}")
endif()

# Expose the canonical target name (only if not already provided by a package)
if (NOT TARGET Eigen3::Eigen)
  add_library(Eigen3::Eigen ALIAS eigen3_headers)
endif()


# ----------------------------------------------------------------------------
# 9) System OpenGL
# ----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)

# ----------------------------------------------------------------------------
# 10) Executable target
# ----------------------------------------------------------------------------
# Collect all .cpp/.cxx sources automatically
file(GLOB_RECURSE DKVIEWER_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
)
#add_executable(dkViewer ${DKVIEWER_SOURCES})
# Filter out main.cpp from the core library sources
list(FILTER DKVIEWER_SOURCES EXCLUDE REGEX "main\\.cpp$")

add_library(dkViewerCore STATIC ${DKVIEWER_SOURCES})

# ----------------------------------------------------------------------------
# 10) Core Library Target
# ----------------------------------------------------------------------------
# Collect all .cpp/.cxx sources automatically
file(GLOB_RECURSE DKVIEWER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
)

# Filter out main.cpp from the core library sources
list(FILTER DKVIEWER_SOURCES EXCLUDE REGEX "main\\.cpp$")

add_library(dkViewerCore STATIC ${DKVIEWER_SOURCES})

# ------------------------------------------------------------------------
# 10.a) Treat include/ headers as part of the project
# ------------------------------------------------------------------------
# Collect all public headers so VS shows them under "Header Files"
file(GLOB_RECURSE DKVIEWER_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

# Add headers to the target (won't compile them)
target_sources(dkViewerCore PRIVATE ${DKVIEWER_HEADERS})

# Organize the headers in Visual Studio
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${DKVIEWER_HEADERS})

# Expose assets directory to code via compile definition
set(ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")
target_compile_definitions(dkViewerCore PRIVATE ASSETS_DIR="${ASSETS_DIR}")

target_include_directories(dkViewerCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(dkViewerCore PUBLIC
    glfw
    glad
    OpenGL::GL
    imgui
    assimp::assimp
    Eigen3::Eigen
)

# ----------------------------------------------------------------------------
# 11) User Interface Executable
# ----------------------------------------------------------------------------
add_executable(dkViewer src/main.cpp)

target_compile_definitions(dkViewer PRIVATE ASSETS_DIR="${ASSETS_DIR}")

target_link_libraries(dkViewer PRIVATE
    dkViewerCore
)

# ----------------------------------------------------------------------------
# 12) Include assets in the VS project and copy at build
# ----------------------------------------------------------------------------
# Gather all asset files so they appear under "Assets" in Solution Explorer
file(GLOB_RECURSE DKVIEWER_ASSETS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*"
)

# Exclude .obj files (to avoid treating them as object files during linking)
list(FILTER DKVIEWER_ASSETS EXCLUDE REGEX "\.obj$")

# Add assets to the dkViewer target so they show up in the IDE (they are not compiled)
if(DKVIEWER_ASSETS)
    target_sources(dkViewer PRIVATE ${DKVIEWER_ASSETS})

    # Group assets under a virtual folder
    source_group("Assets" FILES ${DKVIEWER_ASSETS})
endif()

# Copy assets to the build dir at build time
add_custom_command(TARGET dkViewer PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)

# ----------------------------------------------------------------------------
# 13) Testing
# ----------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)