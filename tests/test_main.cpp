#include "gtest/gtest.h"
#include <string>
#include <filesystem>
#include <random>
#include <limits>
#include "Octree.h"
#include "Mesh.h"


TEST(MeshTests, MeshLoad) {
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

TEST(OctreeTests, BasicOctree)
{
    // Some basic data
    int numPoints = 10;
    std::vector<float> points = GeneratePointsInSphere(numPoints, 4.0f);
    // We need to find the  min/max of X,Y, and Z
    float minX =  std::numeric_limits<float>::infinity();
    float minY =  std::numeric_limits<float>::infinity();
    float minZ =  std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    float maxZ = -std::numeric_limits<float>::infinity();

    // use vertex bounds to define the AABB cell
    for (int id = 0; id < numPoints; id++)
    {
        if (points[id*3 + 0] < minX) minX = points[id*3 + 0];
        if (points[id*3 + 0] > maxX) maxX = points[id*3 + 0];
        if (points[id*3 + 1] < minY) minY = points[id*3 + 1];
        if (points[id*3 + 1] > maxY) maxY = points[id*3 + 1];
        if (points[id*3 + 2] < minZ) minZ = points[id*3 + 2];
        if (points[id*3 + 2] > maxZ) maxZ = points[id*3 + 2];
    }
    float width = maxX - minX + 0.01f;
    float height = maxY - minY + 0.01f;
    float depth = maxZ - minZ + 0.01f;
    // Define the corner
    float x, y, z;
    x = minX - 0.005f;
    y = minY - 0.005f;
    z = minZ - 0.005f;
    bool allElementsWork = true;
    Octree testOctree(points.data(), numPoints, width, height, depth, x, y, z, 1, 4);
    for (int i=0; i< testOctree.cells.size(); i++)
    {
        Cell* cell = &testOctree.cells[i];
        // We need to write this test to make sure the points we find in the cells are
        // actually within the bounds of the 
        float3 cellCenter(cell->pos.x + cell->width / 2.0f,
                          cell->pos.y + cell->height / 2.0f,
                          cell->pos.z + cell->width / 2.0f);
        // List elements and positions
        if (cell->elementId != -1)
        {
            Element* currElem = &testOctree.elements[cell->elementId];
            float3 elemPos;
            elemPos.x = points[currElem->id * 3];
            elemPos.y = points[currElem->id * 3 + 1];
            elemPos.z = points[currElem->id * 3 + 2];
            bool isInRightCell = ((elemPos.x - cell->pos.x) < cell->width &&
                (elemPos.y - cell->pos.y) < cell->height &&
                (elemPos.z - cell->pos.z) < cell->depth);
            while (currElem->nextId != -1)
            {
                currElem = &testOctree.elements[currElem->nextId];
                elemPos.x = points[currElem->id * 3];
                elemPos.y = points[currElem->id * 3 + 1];
                elemPos.z = points[currElem->id * 3 + 2];
                bool isInRightCell = ((elemPos.x - cell->pos.x) < cell->width &&
                    (elemPos.y - cell->pos.y) < cell->height &&
                    (elemPos.z - cell->pos.z) < cell->depth);
                EXPECT_TRUE(isInRightCell);
            }
        }
    }
}
