#include "DKSpringSolver.h"

void DKSpringSolver::step()
{
	if (!doSim) return;
	implicitSolver();
	if (doCollisions)
	{
		detectCollisions();
	}
}

void DKSpringSolver::sparseSetup()
{
	std::vector<FSparseMatrix::Triplet> pat;
	pat.reserve(9 * num_verts + 18 * springs.size());

	// This creates 9 triplets, and puts them into the pat vector. Thus we vectorize the 3x3 matrix
	// and unroll it row-wise.
	auto addFull3x3Pattern = [&](int r0, int c0) {
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c)
			pat.emplace_back(1.0f, r0 + r, c0 + c);
	};

	// Do this for each vertex, along the block diagonal
	for (int i = 0; i < num_verts; ++i) addFull3x3Pattern(3 * i, 3 * i);
	// Do this for each spring, twice
	for (auto& sp : springs) {
		addFull3x3Pattern(3 * sp.edge->a, 3 * sp.edge->b);
		addFull3x3Pattern(3 * sp.edge->b, 3 * sp.edge->a);
	}

	LHS = FSparseMatrix(3 * num_verts, 3 * num_verts);

	LHS.setFromTriplets(pat);
}

void DKSpringSolver::accumulatedFdX()
{
	for (auto& sp : springs)
	{
		FVec x_i(3);
		FVec x_j(3);
		FVec v_i(3);
		FVec v_j(3);
		for (int i = 0; i < 3; ++i)
		{
			x_i[i] = currPos[sp.edge->a * 3 + i];
			x_j[i] = currPos[sp.edge->b * 3 + i];
			v_i[i] = currVel[sp.edge->a * 3 + i];
			v_j[i] = currVel[sp.edge->b * 3 + i];
		}
		FVec n = (x_j - x_i);
		float l = n.length();
		n *= (1.0f/l);
		// TODO: we need a dense matrix here to make this work...
		FMatrix nnt = n.outer(n);
		// spring force
		// The positional derivatives of the spring force, and the spring dampening
		FMatrix K_s{ nnt.rows(), nnt.cols() };
		FMatrix K_d{ nnt.rows(), nnt.cols() };
		K_s.setZero();
		K_d.setZero();
		FMatrix eye = FMatrix::Identity(nnt.rows());
		K_s = -k * (nnt + (l - sp.l0) / l * (eye - nnt));
		FVec b = v_i - v_j;
		K_d = -beta_s / l * ((n.dot(b) * eye + n.outer(b))) * (eye - nnt);
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c) {
				float k_s = K_s(r, c);
				float k_d = K_d(r, c);
				int elem_a_row = sp.edge->a * 3 + r;
				int elem_a_col = sp.edge->a * 3 + c;
				int elem_b_row = sp.edge->b * 3 + r;
				int elem_b_col = sp.edge->b * 3 + c;
				float val_m = -(dt * dt * k_s) - (dt * dt * k_d);
				float val_p = (dt * dt * k_s) + (dt * dt * k_d);
				LHS.setElement(elem_a_row, elem_a_col, LHS(elem_a_row, elem_a_col) + val_m);
				LHS.setElement(elem_b_row, elem_b_col, LHS(elem_b_row, elem_b_col) + val_m);
				LHS.setElement(elem_a_row, elem_b_col, LHS(elem_a_row, elem_b_col) + val_p);
				LHS.setElement(elem_b_row, elem_a_col, LHS(elem_b_row, elem_a_col) + val_p);
			}
		}
	}
}

void DKSpringSolver::accumulatedFdV()
{
	for (auto& sp : springs)
	{
		FVec x_i(3);
		FVec x_j(3);
		for (int i = 0; i < 3; ++i)
		{
			x_i[i] = currPos[sp.edge->a * 3 + i];
			x_j[i] = currPos[sp.edge->b * 3 + i];
		}
		FVec n = (x_j - x_i);
		float l = n.length();
		n *= (1.0f/l);
		FMatrix B = -beta_s * n.outer(n);
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c) {
				int elem_a_row = sp.edge->a * 3 + r;
				int elem_a_col = sp.edge->a * 3 + c;
				int elem_b_row = sp.edge->b * 3 + r;
				int elem_b_col = sp.edge->b * 3 + c;
				float val_m = -(dt * B(r, c));
				float val_p = (dt * B(r, c));
				LHS.setElement(elem_a_row, elem_a_col, LHS(elem_a_row, elem_a_col) + val_m);
				LHS.setElement(elem_b_row, elem_b_col, LHS(elem_b_row, elem_b_col) + val_m);
				LHS.setElement(elem_a_row, elem_b_col, LHS(elem_a_row, elem_b_col) + val_p);
				LHS.setElement(elem_b_row, elem_a_col, LHS(elem_b_row, elem_a_col) + val_p);
			}
		}
	}
}

void DKSpringSolver::reset()
{
	currPos = defaultPos;
	lastPos = defaultPos;
	currVel.setZero();
	F.setZero();
	for (int i = 0; i < num_verts; i++)
	{
		FVec new_pos(3);
		new_pos[0] = currPos[i * 3 + 0];
		new_pos[1] = currPos[i * 3 + 1];
		new_pos[2] = currPos[i * 3 + 2];
		_mesh->SetVertex(new_pos, i);
	}
}

void DKSpringSolver::accumulateForces()
{
	totalE = 0.0f;
	F.setZero();
	FVec G(num_verts * 3);
	for (int i = 0; i < num_verts; ++i)
	{
		G[i * 3 + 1] = -9.8f * (mass / num_verts);
	}
	for (auto& sp : springs)
	{
		FVec x_i(3);
		FVec x_j(3);
		FVec v_i(3);
		FVec v_j(3);
		for (int i = 0; i < 3; ++i)
		{
			x_i[i] = currPos[sp.edge->a * 3 + i];
			x_j[i] = currPos[sp.edge->b * 3 + i];
			v_i[i] = currVel[sp.edge->a * 3 + i];
			v_j[i] = currVel[sp.edge->b * 3 + i];
		}
		FVec n = (x_j - x_i);
		float l = n.length();
		totalE += (l - sp.l0) * (l - sp.l0) * k / 2.0f;
		n *= (1.0f / n.length());
		// spring force
		FVec _f = n * (l - sp.l0) * k;
		// spring dampening
		_f += -beta_s * (n.dot(v_i - v_j)) * n;
		for (int i = 0; i < 3; ++i)
		{
			F[sp.edge->a * 3 + i] += _f[i];
			F[sp.edge->b * 3 + i] += -_f[i];
		}
	}
	F += G;
	F *= globalScale;
	for (int i = 0; i < 3; ++i)
	{
		F[263 * 3 + i] = 0.0f;
		F[275 * 3 + i] = 0.0f;
	}
}

void DKSpringSolver::implicitSolver()
{
	// Implicit Euler
	// TODO: I am currently doing one step at each frame. This makes smaller steps look slower.
	//       I would like to make all steps move more or less at the same speed, but, with a
	//       different accuracy that depends on the step size.
	LHS.setZero(); // We need to zero out the matrix but NOT destroy the pattern!
	// Set the mass to the main sparse matrix
	for (int i = 0; i < num_verts; ++i) {
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c){
				LHS.setElement(i * 3 + r, i * 3 + c, M(r, c));
			}
		}
	}
	currPos = currPos + dt * currVel;
	accumulateForces();
	accumulatedFdX();
	accumulatedFdV();
	FVec nextVel_i = currVel;
	// TODO: This can be optimized as we are making a bunch of memory copies...
	FVec RHS = (- 1 * M) * (nextVel_i - currVel) + dt * (F - beta_g * currVel);
	FVec dv(currPos.size());
	int num_iters = SolveCG(LHS, RHS, dv);
	// Pin the corner verts
	for (int i = 0; i < 3; ++i)
	{
		dv[263 * 3 + i] = 0.0f;
		dv[275 * 3 + i] = 0.0f;
	}
	currVel += dv;
	currPos += dt * currVel;
	for (int i = 0; i < num_verts; i++)
	{
		FVec new_pos(3);
		for(int j=0; j<3; ++j) new_pos[j] = currPos[i * 3 + j];
		_mesh->SetVertex(new_pos, i);
	}
}


bool DKSpringSolver::setup(const std::shared_ptr<Mesh> mesh)
{
	_mesh = mesh;
	springs.clear();
	num_verts = _mesh->GetNumVerts();
	currPos = FVec(3 * num_verts);
	defaultPos = FVec(3 * num_verts);
	currVel = FVec(3 * num_verts);
	F = FVec(3 * num_verts);
	dv = FVec(3 * num_verts);
	M = FSparseMatrix::Identity(3 * num_verts);
	M_inv = M * 1.0f * (1.0 / (mass / num_verts));
	M *= (mass / num_verts);
	// Create a spring for each edge
	for (auto& edge : _mesh->m_edges)
	{
		Spring _sp(edge);
		FVec x_i, x_j;
		_mesh->GetVertex(edge.a, x_i);
		_mesh->GetVertex(edge.b, x_j);
		_sp.l0 = (x_i - x_j).length();
		springs.push_back(_sp);
		for (int i = 0; i < 3; ++i)
		{
			currPos[edge.a * 3 + i] = x_i[i];
			currPos[edge.b * 3 + i] = x_j[i];
		}
	};
	sparseSetup();
	defaultPos = currPos;
	lastPos = currPos;
	return true;
}

bool DKSpringSolver::triIntersect(const Eigen::Vector3f& src,
	const Eigen::Vector3f& vtxA,
	const Eigen::Vector3f& vtxB,
	const Eigen::Vector3f& vtxC,
	const Eigen::Vector3f& tNorm,
	Eigen::Vector3f& hitPoint,
	float tolerance)
{
	Eigen::Vector3f result(0, 0, 0);

	// One of the verts plus the normal define the plane. Compute closest point on plane
	Eigen::Vector3f pToVtx = vtxA - src;
	float distToPlane = tNorm.dot(pToVtx);

	if (abs(distToPlane) > tolerance) return false;

	result = src + tNorm * (distToPlane);

	// Determine if point is inside the triangle
	bool inside = false;

	Eigen::Vector3f alpha = (vtxB - vtxA).cross(result - vtxA);
	Eigen::Vector3f beta = (vtxC - vtxB).cross(result - vtxB);
	Eigen::Vector3f gamma = (vtxA - vtxC).cross(result - vtxC);

	inside = (alpha.dot(tNorm) > 0 && beta.dot(tNorm) > 0 && gamma.dot(tNorm) > 0);

	if (inside)
		hitPoint = result;
	return inside;
}

void DKSpringSolver::detectCollisions()
{

}

void DKSpringSolver::addCollider(const std::shared_ptr<Mesh> m)
{
	colliders.push_back(m);
}

int DKSpringSolver::getNumVerts() {
	return num_verts;
}

const FSparseMatrix* DKSpringSolver::getLHSMtx() {
	return &LHS;
}

const FSparseMatrix* DKSpringSolver::getMassMtx() {
	return &M;
}
const FSparseMatrix* DKSpringSolver::getInvMassMtx() {
	return &M_inv;
}
const FSparseMatrix* DKSpringSolver::getdFdXMtx() {
	return &dFdX;
}
const FSparseMatrix* DKSpringSolver::getdFdVMtx() {
	return &dFdV;
}
const FVec* DKSpringSolver::getDefaultPos() {
	return &defaultPos;
}
const FVec* DKSpringSolver::getCurrPos() {
	return &currPos;
}
const FVec* DKSpringSolver::getCurrVel() {
	return &currVel;
}
const FVec* DKSpringSolver::getF() {
	return &F;
}