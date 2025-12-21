#include "gtest/gtest.h"
#include <string>
#include <filesystem>
#include "Octree.h"
#include "Mesh.h"

TEST(GlobalSetup, BasicAssertion) {
    EXPECT_TRUE(true);
}

TEST(MeshLoad, BasicAssertion) {
    static const std::string g_assets_folder = "D:/Personal/git/dkViewer/assets";
    auto modelPath = std::filesystem::path(g_assets_folder) / "sphere.obj";
    Mesh testMesh = Mesh();
    EXPECT_TRUE(testMesh.LoadFileTinyObj(modelPath.string().c_str(), false));
}
