#include "gtest/gtest.h"
#include <string>
#include <filesystem>
#include <random>
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


std::vector<float> GeneratePointsInSphere(int N, float radius) {
    std::vector<float> points;
    points.reserve(N * 3); // IMPORTANT: Reserve space for 3 floats per point

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-radius, radius);

    float radiusSq = radius * radius;
    int pointsFound = 0;

    while (pointsFound < N) {
        float x = dis(gen);
        float y = dis(gen);
        float z = dis(gen);

        if ((x * x + y * y + z * z) <= radiusSq) {
            // Push 3 floats sequentially
            points.push_back(x);
            points.push_back(y);
            points.push_back(z);
            pointsFound++;
        }
    }

    return points;
}

TEST(BasicOctree, OctreeTests)
{
    // Some basic data
    std::vector<float> points = GeneratePointsInSphere(10, 4.0f);
    float width, height, depth = 4.0f;
    Octree testOctree(points.data(), points.size(), width, height, depth, 1, 4);
    for (auto elem : testOctree.cells)
    {
        // We need to write this test to make sure the points we find in the cells are
        // actually within the bounds of the cells
    }
    EXPECT_TRUE(true);
}
