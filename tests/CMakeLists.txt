include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)


# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(dkViewerTests
    test_main.cpp
)

target_compile_definitions(dkViewerTests PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

target_link_libraries(dkViewerTests PRIVATE
    dkViewerCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(dkViewerTests)

# --- Test Executable 2: Springs ---
add_executable(SpringTests test_spring.cpp)

target_compile_definitions(SpringTests PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

target_link_libraries(SpringTests PRIVATE
    dkViewerCore
    GTest::gtest_main
)

gtest_discover_tests(SpringTests)

# --- Test Executable 3: Speed tests ---
add_executable(SpeedTests test_speed.cpp)

target_compile_definitions(SpeedTests PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

target_link_libraries(SpeedTests PRIVATE
    dkViewerCore
)
