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

target_link_libraries(dkViewerTests PRIVATE
    dkViewerCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(dkViewerTests)
