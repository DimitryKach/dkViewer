#include "gtest/gtest.h"
#include <string>
#include <filesystem>
#include <random>
#include <limits>
#include "Octree.h"
#include "Mesh.h"
#include "LinAlgLib.h"


TEST(MeshTests, MeshLoad) {
    auto modelPath = std::filesystem::path(ASSETS_DIR) / "sphere.obj";
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

TEST(LinearAlgebraTests, VectorTestDot)
{
    DVec testA{ 1.0,2.0,3.0 };
    DVec testB{ 1.0,2.0,3.0 };

    double out = testA.dot(testB);
    EXPECT_DOUBLE_EQ(out, 14.0);
}

TEST(LinearAlgebraTests, VectorTestPlus)
{
    DVec testA{ 1.0,2.0,3.0 };
    DVec testB{ 1.0,2.0,3.0 };

    DVec out = testA+testB;

    EXPECT_DOUBLE_EQ(out[0], 2.0);
    EXPECT_DOUBLE_EQ(out[1], 4.0);
    EXPECT_DOUBLE_EQ(out[2], 6.0);
}

TEST(LinearAlgebraTests, VectorTestMinus)
{
    DVec testA{ 1.0,2.0,3.0 };
    DVec testB{ 1.0,2.0,3.0 };

    testB[0] = 2.0;
    std::cout << testB << std::endl;

    DVec out = testA - testB;
    EXPECT_DOUBLE_EQ(out[0], -1.0);
    EXPECT_DOUBLE_EQ(out[1], 0.0);
    EXPECT_DOUBLE_EQ(out[2], 0.0);
}

TEST(LinearAlgebraTests, VectorTestScalarMul)
{
    DVec testA{ 1.0,2.0,3.0 };

    testA = testA * 2.0;

    EXPECT_DOUBLE_EQ(testA[0], 2.0);
    EXPECT_DOUBLE_EQ(testA[1], 4.0);
    EXPECT_DOUBLE_EQ(testA[2], 6.0);
}

TEST(LinearAlgebraTests, VectorTestCopy)
{
    DVec testA{ 1.0,2.0,3.0 };
    DVec testB;

    testB = testA;
    std::cout << testB << std::endl;

    EXPECT_DOUBLE_EQ(testA[0], testB[0]);
    EXPECT_DOUBLE_EQ(testA[1], testB[1]);
    EXPECT_DOUBLE_EQ(testA[2], testB[2]);
}

TEST(LinearAlgebraTests, MatrixTestCreate)
{
    std::vector<double> vals{ 3,5,7,1 };
    std::vector<int> col_inds{ 1,2,0,2 };
    std::vector<int> row_ptrs{ 0,2,3,4 };

    SparseMatrix spMtx(vals, col_inds, row_ptrs);

    std::cout << spMtx << std::endl;

    EXPECT_TRUE(true);
}

TEST(LinearAlgebraTests, MatrixTestMMul)
{
    std::vector<double> vals{ 0.5,0.5,0.5 };
    std::vector<int> col_inds{ 0,1,2 };
    std::vector<int> row_ptrs{ 0,1,2,3 };

    SparseMatrix spMtx(vals, col_inds, row_ptrs);

    DVec vec{ 1.0, 2.0, 3.0 };
    vec = spMtx * vec;

    EXPECT_TRUE(vec[0] == 0.5 && vec[1] == 1.0 && vec[2] == 1.5);
}

TEST(LinearAlgebraTests, SolveCGTest)
{
    DVec b{ 1.0,0.0,1.0 };
    DVec x{ 0.0, 0.0, 0.0 };
    std::vector<double> vals{ 2.0, -1.0, -1.0, 2.0, -1.0, -1.0, 2.0 };
    std::vector<int> col_inds{ 0, 1, 0, 1, 2, 1, 2 };
    std::vector<int> row_ptrs{ 0, 2, 5, 7 };
    SparseMatrix A(vals, col_inds, row_ptrs);

    int numIters = SolveCG(A, b, x);

    std::cout << "CGSolve result: " << x << std::endl;
    std::cout << "The CGSolve took " << numIters << " iterations." << std::endl;

    EXPECT_NEAR(x[0], 1.0, 1.0e-5);
    EXPECT_NEAR(x[1], 1.0, 1.0e-5);
    EXPECT_NEAR(x[2], 1.0, 1.0e-5);
}

TEST(LinearAlgebraTests, SolveCGTestN50)
{
    auto Make1DLaplacian = [](int N) -> SparseMatrix {
        std::vector<double> vals;
        std::vector<int> cols;
        std::vector<int> row_ptrs;

        row_ptrs.push_back(0); // Start of Row 0

        for (int r = 0; r < N; ++r) {
            // Left Neighbor (if not first node)
            if (r > 0) {
                vals.push_back(-1.0);
                cols.push_back(r - 1);
            }

            // Diagonal (The Node itself)
            vals.push_back(2.0);
            cols.push_back(r);

            // Right Neighbor (if not last node)
            if (r < N - 1) {
                vals.push_back(-1.0);
                cols.push_back(r + 1);
            }

            // Mark end of this row
            row_ptrs.push_back(vals.size());
        }

        return SparseMatrix(vals, cols, row_ptrs);
    };

    int N = 50; // Size of the system
    SparseMatrix A = Make1DLaplacian(N);

    // 1. Create the "Truth" (Expected Solution)
    //    We want x to be all 1.0s
    DVec x_expected(N);
    for (int i = 0; i < N; ++i) x_expected[i] = 1.0;

    // 2. Generate RHS 'b' based on the Truth
    //    b = A * x_expected
    DVec b = A * x_expected;

    // 3. Setup Guess x (Start at 0)
    DVec x(N); // Defaults to zeros

    // 4. Solve
    //    We expect this to take roughly sqrt(ConditionNumber) iterations.
    //    For N=50 Laplacian, that is usually around 25-30 iterations.
    int iters = SolveCG(A, b, x);

    std::cout << "Stress Test (N=" << N << ") Iterations: " << iters << std::endl;

    // 5. Verification
    //    Ensure it actually worked hard
    EXPECT_GT(iters, 10);

    //    Ensure accuracy
    for (int i = 0; i < N; ++i) {
        EXPECT_NEAR(x[i], x_expected[i], 1e-5);
    }
}
