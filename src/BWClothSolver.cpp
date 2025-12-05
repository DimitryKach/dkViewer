#include "BWClothSolver.h"

void BWClothSolver::accumulateForces()
{

}
void BWClothSolver::accumulatedFdX()
{

}
void BWClothSolver::accumulatedFdV()
{

}


bool BWClothSolver::setup(const std::shared_ptr<Mesh> m)
{
	_mesh = m;
	springs.clear();
	n = _mesh->GetNumVerts();
	currPos = Eigen::VectorXf::Zero(3 * n);
	defaultPos = Eigen::VectorXf::Zero(3 * n);
	currVel = Eigen::VectorXf::Zero(3 * n);
	F = Eigen::VectorXf::Zero(3 * n);
	dv = Eigen::VectorXf::Zero(3 * n);
	M = Eigen::MatrixXf::Identity(3 * n, 3 * n);
	M_inv = M * 1.0f / (mass / n);
	M *= (mass / n);
	// Create a spring for each edge
	for (auto& edge : _mesh->m_edges)
	{
		Spring _sp(edge);
		Eigen::Vector3f x_i = _mesh->GetVertex(edge.a);
		Eigen::Vector3f x_j = _mesh->GetVertex(edge.b);
		_sp.l0 = (x_i - x_j).norm();
		springs.push_back(_sp);
		currPos.segment<3>(edge.a * 3) = x_i;
		currPos.segment<3>(edge.b * 3) = x_j;
	};
	//sparseSetup();
	defaultPos = currPos;
	lastPos = currPos;
	return true;
}