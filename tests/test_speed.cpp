#include "LinAlgLib.h"
#include <chrono>
#include <iostream>
#include <algorithm>
#include "DKSpringSolver.h"
#include "EigenSpringSolver.h"
#include "Mesh.h"
#include <string>
#include <filesystem>

struct gen_rand {
	double range;
public:
	gen_rand(double r = 1.0) : range(r) {}
	double operator()() {
		return (rand() / (double)RAND_MAX) * range;
	}
};

// This is what we use in the dense matrix mul
FMatrix mulFast(const FMatrix& A, const FMatrix& B)
{
	assert(A.cols() == B.rows() && "Dimensionality mismatch between matrices");
	FMatrix out(A.rows(), B.cols());
	// We use the IKJ method
	const float* __restrict A_data = A.data();
	const float* __restrict B_data = B.data();
	float* __restrict C_data = out.data();
	int a_rows = A.rows();
	int a_cols = A.cols();
	int b_rows = B.rows();
	int b_cols = B.cols();
	int c_rows = out.rows();
	int c_cols = out.cols();
	// For each row in A
	for (int i = 0; i < a_rows; ++i)
	{
		// Jump to the row of A
		const float* __restrict A_row = A_data + i * a_cols;
		// Jump to the C row
		float* __restrict C_row = C_data + i * c_cols;
		// Now we need a row of B that will change with each column of A
		for (int k = 0; k < a_cols; ++k)
		{
			const float* __restrict B_row = B_data + k * b_cols;
			float val = A_row[k];
			if (val == 0.0f) continue;
			for (int j = 0; j < b_cols; ++j)
			{
				C_row[j] += B_row[j] * val;
			}
		}
	}
	return out;
}

FMatrix mulSlow(const FMatrix& A, const FMatrix& B)
{
	assert(A.cols() == B.rows() && "Dimensionality mismatch between matrices");
	FMatrix out(A.rows(), B.cols());
	out.setZero();
	for (int r = 0; r < A.rows(); ++r)
	{
		for (int c = 0; c < B.cols(); ++c)
		{
			float val = 0;
			for (int rr = 0; rr < B.rows(); ++rr)
			{
				val += A(r, rr) * B(rr, c);
			}
			out(r, c) = val;
		}
	}
	return out;
}

void runMatrixMulTest()
{
	int A_rows = 512;
	int A_cols = 1024;

	int B_rows = 1024;
	int B_cols = 512;

	int num_items = 512 * 1024;

	std::vector<float> A_data;
	std::vector<float> B_data;
	A_data.reserve(num_items);
	B_data.reserve(num_items);
	std::generate_n(std::back_inserter(A_data), num_items, gen_rand());
	std::generate_n(std::back_inserter(B_data), num_items, gen_rand());

	FMatrix A{ 512, 1024, A_data };
	FMatrix B{ 1024, 512, B_data };

	std::cout << "Running the speed test..." << std::endl;

	// Warm up the data
	FMatrix C = mulSlow(A, B);
	std::cout << "Running the mulSlow..." << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < 5; ++i)
	{
		C = mulSlow(A, B);
	}

	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> ms_double = end - start;

	std::cout << "The result took: " << ms_double.count() << std::endl;
	std::cout << C(0, 0) << std::endl;;

	C.setZero();

	// Warm up the data
	C = A * B;
	std::cout << "Running the mulFast..." << std::endl;
	start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < 5; ++i)
	{
		C = A * B;
	}

	end = std::chrono::high_resolution_clock::now();

	ms_double = end - start;

	std::cout << "The result took: " << ms_double.count() << std::endl;
	std::cout << C(0, 0) << std::endl;
}

void runSpringSpeedTest()
{
	std::cout << "Setting up the spring solver speed test" << std::endl;
	int num_iters = 1000;
	std::cout << "Number of iterations being tested is: " << num_iters << std::endl;
	auto modelPath = std::filesystem::path(ASSETS_DIR) / "plane4.obj";
	std::shared_ptr<Mesh> testMesh = std::make_shared<Mesh>();
	testMesh->LoadFileTinyObj(modelPath.string().c_str(), false);
	DKSpringSolver DKSpSolve{};
	EigenSpringSolver EiSpSolve{};
	DKSpSolve.setup(testMesh);
	EiSpSolve.setup(testMesh);

	std::cout << "Number of springs being simulated is: " << DKSpSolve.getNumSprings() << std::endl;

	// Warm up DK solver
	DKSpSolve.doSim = true;
	DKSpSolve.step();
	std::cout << "Running the DKSpSolve..." << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < num_iters; ++i)
	{
		DKSpSolve.step();
	}
	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> ms_double = end - start;

	std::cout << "The result took: " << ms_double.count() << std::endl;
	std::cout << "Average time per step is: " << ms_double.count() / (double)num_iters << "ms" << std::endl;

	// Reset the mesh and stop the sim
	DKSpSolve.doSim = false;
	DKSpSolve.reset();

	// Warm up the Eigen solver
	EiSpSolve.doSim = true;
	EiSpSolve.step();
	std::cout << "Running the EigenSpSolve..." << std::endl;
	start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < num_iters; ++i)
	{
		EiSpSolve.step();
	}
	end = std::chrono::high_resolution_clock::now();

	ms_double = end - start;

	std::cout << "The result took: " << ms_double.count() << std::endl;
	std::cout << "Average time per step is: " << ms_double.count() / (double)num_iters << "ms" << std::endl;

	// Reset the mesh and stop the sim
	EiSpSolve.doSim = false;
	EiSpSolve.reset();
}

int main() {
	runSpringSpeedTest();

	return 0;
}