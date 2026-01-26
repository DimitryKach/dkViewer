#include "gtest/gtest.h"
#include <string>
#include <filesystem>
#include "DKSpringSolver.h"
#include "Mesh.h"

TEST( BaseTests, SpringSetup )
{
	auto modelPath = std::filesystem::path(ASSETS_DIR) / "plane4.obj";
	std::shared_ptr<Mesh> testMesh = std::make_shared<Mesh>();
	testMesh->LoadFileTinyObj(modelPath.string().c_str(), false);
	DKSpringSolver SpSolve{};
	EXPECT_TRUE(SpSolve.setup(testMesh));
	const FSparseMatrix* LHS = SpSolve.getLHSMtx();
	const FSparseMatrix* M = SpSolve.getMassMtx();
	const FSparseMatrix* M_inv = SpSolve.getInvMassMtx();
	// Check that the mesh was loaded with the right number of verts
	int num_verts = SpSolve.getNumVerts();
	EXPECT_EQ(num_verts, testMesh->GetNumVerts());
	// Check that the dimensions of M and LHS are appropriate
	EXPECT_EQ(M->rows(), num_verts * 3);
	EXPECT_EQ(M->cols(), num_verts * 3);
	EXPECT_EQ(LHS->rows(), num_verts * 3);
	EXPECT_EQ(LHS->cols(), num_verts * 3);
	// Check the mass
	float mass = 1.0f / testMesh->GetNumVerts();
	float inv_mass = testMesh->GetNumVerts();
	for (int i = 0; i < num_verts; ++i)
	{
		EXPECT_FLOAT_EQ( (*M)(i*3, i*3), mass);
		EXPECT_FLOAT_EQ((*M_inv)(i*3, i*3), inv_mass);
		EXPECT_FLOAT_EQ((*M)(i * 3 + 1, i * 3 + 1), mass);
		EXPECT_FLOAT_EQ((*M_inv)(i * 3 + 1, i * 3 + 1), inv_mass);
		EXPECT_FLOAT_EQ((*M)(i * 3 + 2, i * 3 + 2), mass);
		EXPECT_FLOAT_EQ((*M_inv)(i * 3 + 2, i * 3 + 2), inv_mass);
	}
	// Check that the default pose is same as the mesh
	const FVec* defaultPos = SpSolve.getDefaultPos();
	for (int i = 0; i < num_verts; ++i)
	{
		FVec pos;
		testMesh->GetVertex(i, pos);
		EXPECT_FLOAT_EQ((*defaultPos)[i * 3 + 0], pos[0]);
		EXPECT_FLOAT_EQ((*defaultPos)[i * 3 + 1], pos[1]);
		EXPECT_FLOAT_EQ((*defaultPos)[i * 3 + 2], pos[2]);
	}
}