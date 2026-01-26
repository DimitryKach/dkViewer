#pragma once
#include "Mesh.h"
#include "LinAlgLib.h"
#include <vector>
#include <set>
#include "Eigen/SparseCore"
#include "Eigen/SparseLU"
#include "unsupported/Eigen/IterativeSolvers"

class DKSpringSolver {
public:
	struct Spring
	{
		Spring(const Mesh::Edge& e) : edge(&e){}
		const Mesh::Edge* edge;
		float l0;
	};
	DKSpringSolver()
	{
		k = 30.0f;
		dt = 0.001f;
		mass = 1.0f;
		beta_s = 0.05f;
		beta_g = 0.005f;
		globalScale = 1.0f;
		doSim = false;
		doCollisions = false;
		colTol = 0.01f;
		vIters = 20;
		integrator = SolverType::IMPLICIT;
		totalE = 0.0f;
		num_verts = 0;
	}
	~DKSpringSolver() = default;
	void accumulateForces();
	void accumulatedFdXdV();
	void step();
	void sparseSetup();
	void reset();
	void implicitSolver();
	bool setup(const std::shared_ptr<Mesh> m);
	void detectCollisions();
	void addCollider(const std::shared_ptr<Mesh> m);
	int getNumVerts();
	int getNumSprings();
	const FSparseMatrix* getLHSMtx();
	const FSparseMatrix* getMassMtx();
	const FSparseMatrix* getInvMassMtx();
	const FSparseMatrix* getdFdXMtx();
	const FSparseMatrix* getdFdVMtx();
	const FVec* getDefaultPos();
	const FVec* getCurrPos();
	const FVec* getCurrVel();
	const FVec* getF();
	float k;
	float dt;
	float mass;
	float beta_s;
	float beta_g;
	float totalE;
	float globalScale;
	bool doSim;
	bool doCollisions;
	float colTol;
	enum SolverType
	{
		SYMPLECTIC,
		IMPLICIT
	};
	int integrator;
	uint16_t vIters;

private:
	static bool triIntersect(const Eigen::Vector3f& src,
		const Eigen::Vector3f& vtxA,
		const Eigen::Vector3f& vtxB,
		const Eigen::Vector3f& vtxC,
		const Eigen::Vector3f& tNorm,
		Eigen::Vector3f& hitPoint,
		float tolerance);
	std::shared_ptr<Mesh> _mesh;
	std::vector<std::shared_ptr<Mesh>> colliders;
	std::vector<Spring> springs;
	FVec currPos;
	FVec lastPos;
	FVec defaultPos;
	FVec currVel;
	FVec lastVel;
	FVec F;
	FVec dv;
	FSparseMatrix M;
	FSparseMatrix M_inv;
	FSparseMatrix dFdX;
	FSparseMatrix dFdV;
	FSparseMatrix LHS;
	uint16_t num_verts;
	std::vector<int> indexCacheSpring;
	std::vector<int> indexCacheDiagonal;
};