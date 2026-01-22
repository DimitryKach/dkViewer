#pragma once
#include "Mesh.h"
#include "LinAlgLib.h"
#include <vector>
#include <set>
#include "Eigen/SparseCore"
#include "Eigen/SparseLU"
#include "unsupported/Eigen/IterativeSolvers"

struct SimState {
	std::vector<float> positions;
	std::vector<float> velocities;
};

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
		n = 0;
	}
	~DKSpringSolver() = default;
	void accumulateForces();
	void accumulatedFdX();
	void accumulatedFdV();
	void step();
	void sparseSetup();
	void reset();
	void symplecticSolver();
	void implicitSolver();
	bool setup(const std::shared_ptr<Mesh> m);
	void detectCollisions();
	void addCollider(const std::shared_ptr<Mesh> m);
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
	SparseMatrix M;
	Eigen::MatrixXf M_inv;
	Eigen::MatrixXf dFdX;
	Eigen::MatrixXf dFdV;
	Eigen::SparseMatrix<float> LHS;
	Eigen::SparseLU< Eigen::SparseMatrix<float> > lu;
	Eigen::BiCGSTAB<Eigen::SparseMatrix<float>, Eigen::IncompleteLUT<float>> bicg;
	bool analyzed = false;
	uint16_t n;
};