#pragma once
#include "Mesh.h"
#include "Eigen/SparseCore"
#include "Eigen/SparseLU"
#include "unsupported/Eigen/IterativeSolvers"
#include <vector>

class BWClothSolver {
public:
	struct Spring
	{
		Spring(const Mesh::Edge& e) : edge(&e) {}
		const Mesh::Edge* edge;
		float l0;
	};
	BWClothSolver()
	{
		k = 30.0f;
		dt = 0.001f;
		mass = 1.0f;
		beta_s = 0.05f;
		beta_g = 0.005f;
		globalScale = 1.0f;
		doSim = false;
		vIters = 20;
		integrator = SolverType::IMPLICIT;
		n = 0;
	}
	~BWClothSolver() = default;
	void accumulateForces();
	void accumulatedFdX();
	void accumulatedFdV();
	//void step();
	//void sparseSetup();
	//void reset();
	//void implicitSolver();
	bool setup(const std::shared_ptr<Mesh> m);
	float k;
	float dt;
	float mass;
	float beta_s;
	float beta_g;
	float globalScale;
	bool doSim;
	enum SolverType
	{
		SYMPLECTIC,
		IMPLICIT
	};
	int integrator;
	uint16_t vIters;

private:
	std::shared_ptr<Mesh> _mesh;
	std::vector<Spring> springs;
	Eigen::VectorXf currPos;
	Eigen::VectorXf lastPos;
	Eigen::VectorXf defaultPos;
	Eigen::VectorXf currVel;
	Eigen::VectorXf lastVel;
	Eigen::VectorXf F;
	Eigen::MatrixXf M;
	Eigen::MatrixXf M_inv;
	Eigen::MatrixXf dFdX;
	Eigen::MatrixXf dFdV;
	Eigen::VectorXf dv;
	Eigen::SparseMatrix<float> LHS;
	Eigen::SparseLU< Eigen::SparseMatrix<float> > lu;
	Eigen::BiCGSTAB<Eigen::SparseMatrix<float>, Eigen::IncompleteLUT<float>> bicg;
	bool analyzed = false;
	uint16_t n;
};