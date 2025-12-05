#include "SpringSolver.h"

void SpringSolver::accumulateForces()
{
	totalE = 0.0f;
	F.setZero();
	Eigen::VectorXf G = Eigen::Vector3f(0.0f, -9.8 * (mass / n), 0.0f).replicate(n, 1);
	for (auto& sp : springs)
	{
		Eigen::Vector3f x_i = currPos.segment<3>(sp.edge->a * 3);
		Eigen::Vector3f x_j = currPos.segment<3>(sp.edge->b * 3);
		Eigen::Vector3f v_i = currVel.segment<3>(sp.edge->a * 3);
		Eigen::Vector3f v_j = currVel.segment<3>(sp.edge->b * 3);
		Eigen::Vector3f n = (x_j - x_i);
		float l = n.norm();
		totalE += (l - sp.l0) * (l - sp.l0) * k / 2.0f;
		n.normalize();
		// spring force
		Eigen::Vector3f _f = n * (l - sp.l0) * k;
		// spring dampening
		_f += -beta_s * (n.dot(v_i - v_j)) * n;
		F.segment<3>(sp.edge->a * 3) += _f;
		F.segment<3>(sp.edge->b * 3) += -_f;
	}
	F += G;
	F *= globalScale;
	/*F.col(0) = Eigen::Vector3f::Zero();
	F.col(2) = Eigen::Vector3f::Zero();*/
	/*F.col(68) = Eigen::Vector3f::Zero();
	F.col(73) = Eigen::Vector3f::Zero();*/
	F.segment<3>(263 * 3) = Eigen::Vector3f::Zero();
	F.segment<3>(275 * 3) = Eigen::Vector3f::Zero();
	/*F.col(1680) = Eigen::Vector3f::Zero();
	F.col(1269) = Eigen::Vector3f::Zero();*/
	//std::cout << "Total E=" << totalE << std::endl;
}

void SpringSolver::accumulatedFdX()
{
	for (auto& sp : springs)
	{
		Eigen::Vector3f x_i = currPos.segment<3>(sp.edge->a * 3);
		Eigen::Vector3f x_j = currPos.segment<3>(sp.edge->b * 3);
		Eigen::Vector3f v_i = currVel.segment<3>(sp.edge->a * 3);
		Eigen::Vector3f v_j = currVel.segment<3>(sp.edge->b * 3);
		Eigen::Vector3f n = (x_j - x_i);
		float l = n.norm();
		n.normalize();
		Eigen::Matrix3f nnt = n * n.transpose();
		// spring force
		Eigen::Matrix3f K_s, K_d; // The positional derivatives of the spring force, and the spring dampening
		K_s.setZero();
		K_d.setZero();
		K_s = -k * (nnt + (l - sp.l0) / l * (Eigen::Matrix3f::Identity() - nnt));
		Eigen::Vector3f b = v_i - v_j;
		K_d = -beta_s / l * ((n.dot(b) * Eigen::Matrix3f::Identity() + n * b.transpose())) * (Eigen::Matrix3f::Identity() - nnt);
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c) {
				float k_s = K_s(r, c);
				float k_d = K_d(r, c);
				LHS.coeffRef(sp.edge->a * 3 + r, sp.edge->a * 3 + c) += -(dt * dt * k_s) - (dt * dt * k_d);
				LHS.coeffRef(sp.edge->b * 3 + r, sp.edge->b * 3 + c) += -(dt * dt * k_s) - (dt * dt * k_d);
				LHS.coeffRef(sp.edge->a * 3 + r, sp.edge->b * 3 + c) += (dt * dt * k_s) + (dt * dt * k_d);
				LHS.coeffRef(sp.edge->b * 3 + r, sp.edge->a * 3 + c) += (dt * dt * k_s) + (dt * dt * k_d);
			}
		}
	}
}

void SpringSolver::accumulatedFdV()
{
	for (auto& sp : springs)
	{
		Eigen::Vector3f x_i = currPos.segment<3>(sp.edge->a * 3);
		Eigen::Vector3f x_j = currPos.segment<3>(sp.edge->b * 3);
		Eigen::Vector3f n = (x_j - x_i);
		float l = n.norm();
		n.normalize();
		Eigen::Matrix3f B = -beta_s * n * n.transpose();
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c) {
				LHS.coeffRef(sp.edge->a * 3 + r, sp.edge->a * 3 + c) += -(dt * B(r, c));
				LHS.coeffRef(sp.edge->b * 3 + r, sp.edge->b * 3 + c) += -(dt * B(r, c));;
				LHS.coeffRef(sp.edge->a * 3 + r, sp.edge->b * 3 + c) += (dt * B(r, c));;
				LHS.coeffRef(sp.edge->b * 3 + r, sp.edge->a * 3 + c) += (dt * B(r, c));;
			}
		}
	}
}

void SpringSolver::step()
{
	if (!doSim) return;
	switch (integrator) {
	case SolverType::SYMPLECTIC:
		symplecticSolver();
		break;
	case SolverType::IMPLICIT:
		implicitSolver();
		break;
	}
}

void SpringSolver::sparseSetup()
{
	std::vector<Eigen::Triplet<float>> pat;
	pat.reserve(9 * n + 18 * springs.size());

	// This creates 9 triplets, and puts them into the pat vector. Thus we vectorize the 3x3 matrix
	// and unroll it row-wise.
	auto addFull3x3Pattern = [&](int r0, int c0) {
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c)
			pat.emplace_back(r0 + r, c0 + c, 1.0f);
	};

	for (int i = 0; i < n; ++i) addFull3x3Pattern(3 * i, 3 * i); // Do this for each vertex, along the block diagonal
	for (auto& sp : springs) { // Do this for each spring, twice
		addFull3x3Pattern(3 * sp.edge->a, 3 * sp.edge->b);
		addFull3x3Pattern(3 * sp.edge->b, 3 * sp.edge->a);
	}

	LHS = Eigen::SparseMatrix<float>(3 * n, 3 * n);

	LHS.setFromTriplets(pat.begin(), pat.end());
	LHS.makeCompressed();
	assert(LHS.isCompressed());

	bicg.setTolerance(1e-4);
	bicg.setMaxIterations(200);
	bicg.preconditioner().setFillfactor(4);
}

void SpringSolver::reset()
{
	currPos = defaultPos;
	lastPos = defaultPos;
	currVel.setZero();
	F.setZero();
	for (int i = 0; i < n; i++)
	{
		Eigen::Vector3f new_pos = currPos.segment<3>(i * 3);
		_mesh->SetVertex(new_pos, i);
	}
}

void SpringSolver::symplecticSolver()
{
	accumulateForces();
	currVel += M_inv * (dt * (F - beta_g * currVel));
	currPos = currPos + dt * currVel;

	for (int i = 0; i < n; i++)
	{
		Eigen::Vector3f new_pos = currPos.segment<3>(i * 3);
		_mesh->SetVertex(new_pos, i);
	}
}

void SpringSolver::implicitSolver()
{
	// Implicit Euler
	Eigen::Map<Eigen::VectorXf>(LHS.valuePtr(), LHS.nonZeros()).setZero(); // We need to zero out the matrix but NOT destroy the pattern!
	// Set the mass to the main sparse matrix
	for (int i = 0; i < n; ++i) {
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c)
			LHS.coeffRef(i * 3 + r, i * 3 + c) += M(r, c);
	}
	currPos = currPos + dt * currVel;
	accumulateForces();
	accumulatedFdX();
	accumulatedFdV();
	if (!analyzed) { lu.analyzePattern(LHS); analyzed = true; } // once
	Eigen::VectorXf nextVel_i = currVel;
	Eigen::VectorXf RHS = -M * (nextVel_i - currVel) + dt * (F - beta_g * currVel);
	lu.factorize(LHS);
	Eigen::VectorXf dv = lu.solve(RHS);
	//bicg.compute(LHS); // cheap vs LU
	//dv = bicg.solveWithGuess(RHS, dv);
	/*Eigen::VectorXf dv = LHS.fullPivLu().solve(RHS);*/
	dv.segment<3>(263 * 3) = Eigen::Vector3f::Zero();
	dv.segment<3>(275 * 3) = Eigen::Vector3f::Zero();
	currVel += dv;
	currPos += dt * currVel;
	for (int i = 0; i < n; i++)
	{
		Eigen::Vector3f new_pos = currPos.segment<3>(i * 3);
		_mesh->SetVertex(new_pos, i);
	}
}



bool SpringSolver::setup(const std::shared_ptr<Mesh> mesh)
{
	_mesh = mesh;
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
	sparseSetup();
	defaultPos = currPos;
	lastPos = currPos;
	return true;
}