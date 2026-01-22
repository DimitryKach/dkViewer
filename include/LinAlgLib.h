#include <vector>
#include <memory>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <cmath>
#include <stdexcept>


template <typename T>
class Vec {
	std::vector<T> m_data;
public:
	explicit Vec(std::initializer_list<T> list) : m_data(list) {};
	explicit Vec(const std::vector<T>& _data) : m_data(_data) {};
	explicit Vec(size_t N) {
		m_data = std::vector<T>(N, T(0));
	};
	Vec() {};
	~Vec() {};

	void addScaled(const Vec<T>& x, T scalar)
	{
		assert(x.size() == m_data.size() && "The vector you are adding must match in dimensions");
		for (int i = 0; i < m_data.size(); ++i)
		{
			m_data[i] += x[i] * scalar;
		}
	}

	bool operator==(const Vec& a) {
		bool matching = a.m_data.size() == m_data.size();
		if (!matching) return matching;
		for (int i = 0; i < m_data.size(); ++i)
		{
			matching = std::abs(m_data[i] - a.m_data[i]) < 1e-5; // This should work for floats and double
			if (!matching) return matching;
		}
		return matching;
	}

	Vec operator+(const Vec& a) const {
		assert(a.m_data.size() == m_data.size() && "The vector you are adding must match in dimensions");
		Vec out(m_data);
		for (int i = 0; i < m_data.size(); ++i)
		{
			out[i] += a[i];
		}
		return out;
	}

	void operator+=(const Vec& a) {
		assert(a.m_data.size() == m_data.size() && "The vector you are adding must match in dimensions");
		for (int i = 0; i < m_data.size(); ++i)
		{
			m_data[i] += a.m_data[i];
		}
	}

	Vec operator-(const Vec& a) const {
		assert(a.m_data.size() == m_data.size() && "The vector you are subtracting must match in dimensions");
		Vec out(m_data);
		for (int i = 0; i < m_data.size(); ++i)
		{
			out[i] -= a[i];
		}
		return out;
	}

	friend Vec operator*(T scalar, const Vec& vec) {
		Vec out{ vec.m_data };
		for (int i = 0; i < vec.m_data.size(); ++i)
		{
			out.m_data[i] *= scalar;
		}
		return out;
	}

	friend Vec operator*(const Vec& vec, T scalar) {
		return scalar * vec;
	}

	void operator*=(T scalar) {
		for (int i = 0; i < m_data.size(); ++i)
		{
			m_data[i] *= scalar;
		}
	}

	void operator-=(const Vec& a) {
		assert(a.m_data.size() == m_data.size() && "The vector you are subtracting must match in dimensions");
		for (int i = 0; i < m_data.size(); ++i)
		{
			m_data[i] -= a.m_data[i];
		}
	}

	// Note to self - we needed an & because we are returning an lvalue that is a reference instead of a copy.
	T& operator[](size_t i) {
		return m_data[i];
	}
	// need a const accessor when the Vec variable is declared const
	const T& operator[](size_t i) const {
		return m_data[i];
	}

	T dot(const Vec& a) {
		assert(a.m_data.size() == m_data.size() && "The vectors for the inner product must match in dimensions");
		T out = T(0);
		for (int i = 0; i < m_data.size(); ++i)
		{
			out += m_data[i] * a.m_data[i];
		}
		return out;
	}

	size_t size() const {
		return m_data.size();
	}

	void resize(size_t size) {
		m_data.resize(size);
	}

	friend std::ostream& operator <<(std::ostream& out, const Vec& vec)
	{
		out << "\n" << "[";
		for (int i = 0; i < vec.m_data.size(); ++i)
		{
			out << vec.m_data[i];
			if (i + 1 != vec.m_data.size()) out << ", ";
		}
		out << "]\n";
		return out;
	}

	T length() {
		return std::sqrt(lensq());
	}

	T lensq(){
		return this->dot(*this);
	}

};

using DVec = Vec<double>;
using FVec = Vec<float>;

template <typename T>
class SparseMatrix {
	int rows;
	int cols;
	// Compressed Sparse Row format
	std::vector<T> values;
	std::vector<int> col_indices;
	std::vector<int> row_ptrs; // Size is rows + 1

	// TODO: Implement Matrix-Vector Multiply

	friend std::ostream & operator <<(std::ostream& out, const SparseMatrix& mtx)
	{
		out << "\nSparseMatrx " << mtx.rows << "x" << mtx.cols << "\n";
		for (int r = 0; r < mtx.rows; ++r)
		{
			for (int col_start = mtx.row_ptrs[r]; col_start < mtx.row_ptrs[r + 1]; ++col_start)
			{
				out << "(" << r << "," << mtx.col_indices[col_start] << "): " << mtx.values[col_start] << "\n";
			}
		}
		out << "\n";
		return out;
	}
public:
	struct Triplet {
		T value;
		int row;
		int col;
	};

	// Creates a square matrix from the input values
	SparseMatrix(std::vector<T>& vals, std::vector<int>& col_inds, std::vector<int>& row_ps)
	{
		assert(vals.size() == col_inds.size() && "The size of values passed must match the size of column IDs");
		rows = row_ps.size() - 1;
		cols = rows;
		row_ptrs = row_ps;
		values = vals;
		col_indices = col_inds;
	}

	// Creates a square matrix from the input values
	SparseMatrix(std::vector<T>& vals, std::vector<int>& col_inds, std::vector<int>& row_ps, int n_rows, int n_cols)
	{
		assert(vals.size() == col_inds.size() && "The size of values passed must match the size of column IDs");
		assert(n_rows == row_ps.size() - 1 && "The number of rows must be one less than the size of the row pointer vector");
		rows = n_rows;
		cols = n_cols;
		row_ptrs = row_ps;
		values = vals;
		col_indices = col_inds;
	}

	// Create an empty NxN SparseMatrix
	SparseMatrix(int n_rows)
	{
		rows = n_rows;
		cols = n_rows;
		row_ptrs = std::vector<int>(n_rows + 1, 0);
		values = std::vector<T>();
		col_indices = std::vector<int>();
	}

	// Create an empty NxM SparseMatrix
	SparseMatrix(int n_rows, int n_cols)
	{
		rows = n_rows;
		cols = n_cols;
		row_ptrs = std::vector<int>(n_rows + 1, 0);
		values = std::vector<T>();
		col_indices = std::vector<int>();
	}

	~SparseMatrix() {};
	Vec<T> operator*(const Vec<T>& x) const {
		assert(x.size() == static_cast<size_t>(cols) && "The input vector must have the same number of elements as there are columns");
		Vec<T> result(rows);
		for (int r = 0; r < rows; ++r)
		{
			// Suggested accumulator living on register mem
			// TODO: Verify the performance gain here if possible
			T sum = T(0);
			for (int col_start = row_ptrs[r]; col_start < row_ptrs[r + 1]; ++col_start)
			{
				sum += values[col_start] * x[col_indices[col_start]];
			}
			result[r] = sum;
		}
		return result;
	}

	friend SparseMatrix<T> operator*(T scalar, const SparseMatrix<T>& mtx) {
		SparseMatrix<T> out = mtx;
		for (int i=0; i < out.values.size(); ++i)
		{
			out.values[i] *= scalar;
		}
		return out;
	}

	friend SparseMatrix<T> operator*(const SparseMatrix<T>& mtx, T scalar) {
		return scalar * mtx;
	}
	int getRows() {
		return rows;
	}
	const std::vector<T>* getValues() {
		return &values;
	}
	const std::vector<int>* getColIndices() {
		return &col_indices;
	}
	const std::vector<int>* getRowPtrs() {
		return &row_ptrs;
	}
	void setFromTriplets(std::vector<Triplet>& triplets)
	{
		std::sort(triplets.begin(), triplets.end(), [](const Triplet& a, const Triplet& b) {
			// Logic: Return true if 'a' comes before 'b'
			bool out = (a.row < b.row) ? true : ((a.row > b.row) ? false : ((a.col < b.col) ? true : false));
			return out;
		});
		std::vector<T> vals(triplets.size());
		std::vector<int> columns(triplets.size());
		std::vector<int> row_ps(rows + 1, 0);
		row_ps[row_ps.size() - 1] = triplets.size();
		int curr_row = 0;
		for (int i=0; i < triplets.size(); ++i)
		{
			vals[i] = triplets[i].value;
			columns[i] = triplets[i].col;
			// If we switched rows, I need to store in the row_ptrs
			if (curr_row != triplets[i].row)
			{
				if (triplets[i].row > rows - 1)
				{
					throw std::runtime_error("The provided data has rows outside the maximum specified");
				}
				while (curr_row != triplets[i].row)
				{
					++curr_row;
					row_ps[curr_row] = i;
				}
			}
		}
		// Fill the missing row ptrs if there are zero rows at the end
		while (curr_row != row_ps.size()-1)
		{
			++curr_row;
			row_ps[curr_row] = row_ps[row_ps.size() - 1];
		}
		values = std::move(vals);
		col_indices = std::move(columns);
		row_ptrs = std::move(row_ps);
	}
	void mul(const Vec<T>& x, Vec<T>& result) const
	{
		assert(&x != &result && "In-place multiplication is not supported");
		assert(x.size() == static_cast<size_t>(cols) && "The input vector must have the same number of elements as there are columns");
		if (result.size() != rows) result.resize(rows);

		for (int r = 0; r < rows; ++r)
		{
			T sum = T(0);
			for (int col_start = row_ptrs[r]; col_start < row_ptrs[r + 1]; ++col_start)
			{
				sum += values[col_start] * x[col_indices[col_start]];
			}
			result[r] = sum;
		}
	}
};

using DSparseMatrix = SparseMatrix<double>;
using FSparseMatrix = SparseMatrix<float>;

template <typename T>
int SolveCG(const SparseMatrix<T>& A, const Vec<T>& b, Vec<T>& x, int max_iter = 100, T tol = T(1e-6))
{
	Vec<T> r = b - A * x;
	T r0lensq = r.lensq();
	Vec<T> p = r;

	int curr_iter = 0;

	// Worker variables
	Vec<T> Ap;


	while (curr_iter < max_iter && r0lensq > tol * tol)
	{
		A.mul(p, Ap);
		T alpha = r0lensq / p.dot(Ap);
		x.addScaled(p, alpha);
		r.addScaled(Ap, -alpha);
		T r1lensq = r.lensq();
		if (r1lensq < tol * tol) break;
		T beta = r1lensq / r0lensq;
		p *= beta;
		p += r;
		r0lensq = r1lensq;
		++curr_iter;
	}

	return curr_iter;
};