#include "gtest/gtest.h"
#include <string>
#include <filesystem>
#include <random>
#include <algorithm>
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

    const std::vector<double>* p_vals = spMtx.getValues();
    const std::vector<int>* p_colInds = spMtx.getColIndices();
    const std::vector<int>* p_rowPtrs = spMtx.getRowPtrs();

    for (int i = 0; i < vals.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(vals[i], p_vals->at(i));
    }
    for (int i = 0; i < col_inds.size(); ++i)
    {
        EXPECT_EQ(col_inds[i], p_colInds->at(i));
    }
    for (int i = 0; i < row_ptrs.size(); ++i)
    {
        EXPECT_EQ(row_ptrs[i], p_rowPtrs->at(i));
    }
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

    EXPECT_NEAR(x[0], 1.0, 1.0e-5);
    EXPECT_NEAR(x[1], 1.0, 1.0e-5);
    EXPECT_NEAR(x[2], 1.0, 1.0e-5);
}

TEST(LinearAlgebraTests, SolveCGTestN50)
{
    auto Make1DLaplacian = [](int N) -> DSparseMatrix {
        // Think of a system of equations for a line of connected springs
        // |-----o-----o----o------o-----|  |-----> x
        // Label each spring as a mass m_i, so m_0, m_1, m_2, etc.,
        // and label the displacements from the rest state as - x_0, x_1, ...
        // Now let's figure out the forces. Recall F_spring = kx, where x is the displacement from rest.
        // Assume k=1, and let the rest lengths be all 0. Then, let's find our forces.
        // For index 0 we have F_0 = -kx_0 + k(x_1-x_0) = -2kx_0 + kx_1
        // For index 1 we have F_1 = -k(x_1-x_0) + k(x_2 - x_1) = kx_0 - 2kx_1 + kx_2
        // For index 2 we have F_2 = -k(x_2-x_1) + k(x_3 - x_2) = kx_1 - 2kx_2 + kx_3
        // ...
        // For index N we have F_0 = -kx_N - k(x_N-x_(N-1)) = kx_(N-1) - 2kx_N.
        // Now let's see if we can make that into a matrix and vector output!
        // Let X = {x_0, x_1, x_2, ..., x_N}, and F be a NxN matrix
        // Then F[0][0] = -2, F[0][1] = 1, and F[0][2...N] = 0
        // F[1][0] = 1, F[1][1] = -2, F[1][2] = 1, F[1][3...N] = 0,
        // F[2][0] = 0, F[2][1] = 1, F[2][2] = -2, F[2][3] = 1, F[2][4...N] = 0,
        // ...
        // F[N][0...N-2] = 0, F[N][N-1] = 1, F[N][N] = -2.
        // That's the matrix we construct.
        std::vector<double> vals;
        std::vector<int> cols;
        // We know the size of row_ptrs, so we can pre-allocate
        std::vector<int> row_ptrs(N+1, 0);

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
            row_ptrs[r+1] = vals.size();
        }

        return DSparseMatrix(vals, cols, row_ptrs);
    };

    int N = 50; // Size of the system
    DSparseMatrix A = Make1DLaplacian(N);

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

    // 5. Verification
    //    Ensure it actually worked hard
    EXPECT_GT(iters, 10);

    //    Ensure accuracy
    for (int i = 0; i < N; ++i) {
        EXPECT_NEAR(x[i], x_expected[i], 1e-5);
    }
}

TEST(LinearAlgebraTests, SparseMatrixConstructFromTriplets)
{
    std::vector<double> vals{ 2.0, -1.0, -1.0, 2.0, -1.0, 1.0 };
    std::vector<int> col_inds{ 0, 1, 0, 1, 2, 3 };
    std::vector<int> row_ptrs{ 0, 0, 2, 2, 6 };
    
    std::vector<SparseMatrix<double>::Triplet> triplets(vals.size());

    for (int r = 0; r < row_ptrs.size()-1; ++r)
    {
        for (int val_idx = row_ptrs[r]; val_idx < row_ptrs[r + 1]; ++val_idx)
        {
            triplets[val_idx].col = col_inds[val_idx];
            triplets[val_idx].row = r;
            triplets[val_idx].value = vals[val_idx];
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(triplets.begin(), triplets.end(), g);

    DSparseMatrix mtx(4);
    mtx.setFromTriplets(triplets);

    const std::vector<double>* p_vals = mtx.getValues();
    const std::vector<int>* p_colInds = mtx.getColIndices();
    const std::vector<int>* p_rowPtrs= mtx.getRowPtrs();

    for (int i = 0; i < vals.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(vals[i], p_vals->at(i));
    }
    for (int i = 0; i < col_inds.size(); ++i)
    {
        EXPECT_EQ(col_inds[i], p_colInds->at(i));
    }
    for (int i = 0; i < row_ptrs.size(); ++i)
    {
        EXPECT_EQ(row_ptrs[i], p_rowPtrs->at(i));
    }
}

TEST(LinearAlgebraTests, SparseMatrixFromTripletsZeroLastRow)
{
    // 1 0 0
    // 0 1 0
    // 0 0 0
    std::vector<double> vals{ 1.0, 1.0 };
    std::vector<int> col_inds{ 0, 1};
    std::vector<int> row_ptrs{ 0, 1, 2, 2 };

    std::vector<SparseMatrix<double>::Triplet> triplets(vals.size());

    for (int r = 0; r < row_ptrs.size() - 1; ++r)
    {
        for (int val_idx = row_ptrs[r]; val_idx < row_ptrs[r + 1]; ++val_idx)
        {
            triplets[val_idx].col = col_inds[val_idx];
            triplets[val_idx].row = r;
            triplets[val_idx].value = vals[val_idx];
        }
    }

    DSparseMatrix mtx(3);
    mtx.setFromTriplets(triplets);

    const std::vector<double>* p_vals = mtx.getValues();
    const std::vector<int>* p_colInds = mtx.getColIndices();
    const std::vector<int>* p_rowPtrs = mtx.getRowPtrs();

    for (int i = 0; i < vals.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(vals[i], p_vals->at(i));
    }
    for (int i = 0; i < col_inds.size(); ++i)
    {
        EXPECT_EQ(col_inds[i], p_colInds->at(i));
    }
    for (int i = 0; i < row_ptrs.size(); ++i)
    {
        EXPECT_EQ(row_ptrs[i], p_rowPtrs->at(i));
    }
}

TEST(LinearAlgebraTests, SparseMatrixIdentity)
{
    DSparseMatrix eye = DSparseMatrix::Identity(100);
    auto values = eye.getValues();

    for (int i = 0; i < 100; ++i)
    {
        EXPECT_DOUBLE_EQ(values->at(i), 1.0);
    }
}

TEST(LinearAlgebraTests, SparseMatrixElementFuncs)
{
    DSparseMatrix eye = DSparseMatrix::Identity(100);

    eye.setElement(10, 10, 2.0);
    auto values = eye.getValues();
    // Check if the value was set correctly
    EXPECT_DOUBLE_EQ(values->at(10), 2.0);
    // check if we can fetch the value with the () operator
    EXPECT_DOUBLE_EQ(eye(10, 10), 2.0);
    // Make sure we get a 0 if we parse a non-pattern element
    EXPECT_DOUBLE_EQ(eye(10, 11), 0.0);
    // Make sure we cannot set the non-pattern element
    EXPECT_THROW(eye.setElement(10, 11, 2.0), std::runtime_error);
    eye.setZero();
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_DOUBLE_EQ(values->at(i), 0.0);
    }
}

TEST(LinearAlgebraTests, MatrixTranspose)
{
    // TODO: make this a generic test that makes random matrices
    std::vector<double> data{ 0,1,0,2,3,1,0,1,1,4,3,2,1,0,1 };
    DMatrix test(5, 3, data);
    // Check that the values and storage match
    for (int r = 0; r < 5; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_DOUBLE_EQ(test(r, c), data[r * 3 + c]);
        }
    }
    // Test transposing
    DMatrix testT = test.transpose();
    for (int r = 0; r < 5; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_DOUBLE_EQ(testT(c, r), test(r, c));
        }
    }
}

TEST(LinearAlgebraTests, MatrixMul)
{
    // TODO: make this a generic test that makes random matrices
    std::vector<double> data{ 1,2,3,4,5,6 };
    DMatrix a(2, 3, data);
    DMatrix b(3, 2, data);

    DMatrix result = a * b;
    // Check dimensions of result
    EXPECT_EQ(result.rows(), 2);
    EXPECT_EQ(result.cols(), 2);
    // Check that the results are correct
    EXPECT_DOUBLE_EQ(result(0, 0), 22);
    EXPECT_DOUBLE_EQ(result(0, 1), 28);
    EXPECT_DOUBLE_EQ(result(1, 0), 49);
    EXPECT_DOUBLE_EQ(result(1, 1), 64);

    result *= -1;
    EXPECT_DOUBLE_EQ(result(0, 0), -22);
    EXPECT_DOUBLE_EQ(result(0, 1), -28);
    EXPECT_DOUBLE_EQ(result(1, 0), -49);
    EXPECT_DOUBLE_EQ(result(1, 1), -64);

    result = result * 2.0;
    EXPECT_DOUBLE_EQ(result(0, 0), -44);
    EXPECT_DOUBLE_EQ(result(0, 1), -56);
    EXPECT_DOUBLE_EQ(result(1, 0), -98);
    EXPECT_DOUBLE_EQ(result(1, 1), -128);
}

TEST(LinearAlgebraTests, MatrixCompare)
{
    std::vector<double> data{ 1,2,3,4 };
    DMatrix a(2, 2, data);
    DMatrix b(2, 2, data);

    EXPECT_TRUE(a == b);

    b *= 2.0;

    EXPECT_FALSE(a == b);
}

TEST(LinearAlgebraTests, MatrixAddSub)
{
    // TODO: make this a generic test that makes random matrices
    std::vector<double> data{ 1,2,3,4 };
    DMatrix a(2, 2, data);
    DMatrix b(2, 2, data);

    b *= 2.0f;

    DMatrix result = b - a;
    // Check that the results are correct
    EXPECT_DOUBLE_EQ(result(0, 0), 1);
    EXPECT_DOUBLE_EQ(result(0, 1), 2);
    EXPECT_DOUBLE_EQ(result(1, 0), 3);
    EXPECT_DOUBLE_EQ(result(1, 1), 4);
}
