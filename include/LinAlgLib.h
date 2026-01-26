#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include <numeric>
#include <initializer_list>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

template <typename T> class Matrix;

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
			// TODO: the tolerance is hardcoded. We should make a more flexible way of handling it.
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

	void setZero() {
		std::fill(m_data.begin(), m_data.end(), T(0));
	}

	Matrix<T> outer(const Vec& vec) const {
		assert(vec.m_data.size() == m_data.size());
		int n_rows = m_data.size();
		Matrix<T> out(n_rows, n_rows);
		T* __restrict mtx_data = out.data();
		const T* __restrict A_data = m_data.data();
		const T* __restrict B_data = vec.m_data.data();
		for (int r = 0; r < n_rows; ++r)
		{
			const T A_val = A_data[r];
			T* __restrict row_data = mtx_data + r * n_rows;
			for (int c = 0; c < n_rows; ++c)
			{
				row_data[c] = A_val * B_data[c];
			}
		}
		return out;
	}
};

using DVec = Vec<double>;
using FVec = Vec<float>;

template <typename T> class Matrix3;

template <typename T>
class Vec3 {
	T m_data[3];
public:
	explicit Vec3() : m_data{ 0 } {};
	explicit Vec3(T val) : m_data{ val,val,val} {};
	explicit Vec3(T x, T y, T z) : m_data{x,y,z} {};

	bool operator==(const Vec3<T>& vec) const {
		for (int i = 0; i < 3; ++i)
		{
			if (std::abs(m_data[i] - vec.m_data[i]) > 1e-5)
				return false;
		}
		return true;
	}

	Vec3<T> operator+(const Vec3<T>& vec) const {
		Vec3<T> out{ m_data[0] + vec.m_data[0],
					 m_data[1] + vec.m_data[1],
					 m_data[2] + vec.m_data[2] };
		return out;
	}

	void operator+=(const Vec3<T>& vec) {
		m_data[0] += vec.m_data[0];
		m_data[1] += vec.m_data[1];
		m_data[2] += vec.m_data[2];
	}

	Vec3<T> operator-(const Vec3<T>& vec) const {
		Vec3<T> out{ m_data[0] - vec.m_data[0],
					 m_data[1] - vec.m_data[1],
					 m_data[2] - vec.m_data[2] };
		return out;
	}

	friend Vec3<T> operator*(T scalar, const Vec3<T>& vec) {
		Vec3<T> out{ vec.m_data[0]*scalar, vec.m_data[1]*scalar, vec.m_data[2]*scalar};
		return out;
	}
	
	friend Vec3<T> operator*(const Vec3<T>& vec, T scalar) {
		return scalar * vec;
	}

	void operator*=(T scalar) {
		m_data[0] *= scalar;
		m_data[1] *= scalar;
		m_data[2] *= scalar;
	}

	void operator-=(const Vec3<T>& vec) {
		m_data[0] -= vec.m_data[0];
		m_data[1] -= vec.m_data[1];
		m_data[2] -= vec.m_data[2];
	}

	// Note to self - we needed an & because we are returning an lvalue that is a reference instead of a copy.
	T& operator[](size_t i) {
		return m_data[i];
	}
	// need a const accessor when the Vec variable is declared const
	const T& operator[](size_t i) const {
		return m_data[i];
	}

	T dot(const Vec3<T>& vec) const {
		T out = T(0);
		out += m_data[0] * vec.m_data[0];
		out += m_data[1] * vec.m_data[1];
		out += m_data[2] * vec.m_data[2];
		return out;
	}

	size_t size() const {
		return 3;
	}

	friend std::ostream& operator <<(std::ostream& out, const Vec3<T>& vec)
	{
		out << "\n" << "[";
		for (int i = 0; i < 3; ++i)
		{
			out << vec.m_data[i];
			if (i != 2) out << ", ";
		}
		out << "]\n";
		return out;
	}

	T length() const {
		return std::sqrt(lensq());
	}

	T lensq() const {
		return this->dot(*this);
	}

	void setZero() {
		std::fill(m_data, m_data + 3, T(0));
	}

	Matrix3<T> outer(const Vec3<T>& vec) const;
};

using DVec3 = Vec3<double>;
using FVec3 = Vec3<float>;

template <typename T>
class SparseMatrix {
	int m_rows;
	int m_cols;
	// Compressed Sparse Row format
	std::vector<T> values;
	std::vector<int> col_indices;
	std::vector<int> row_ptrs; // Size is rows + 1
public:
	struct Triplet {
		Triplet(T val, int r, int c) : value(val), row(r), col(c) {};
		Triplet() : value(T(0)), row(-1), col(-1) {};
		~Triplet() {};
		T value;
		int row;
		int col;
	};

	static SparseMatrix<T> Identity(int num_rows)
	{
		SparseMatrix<T> out{};
		out.m_rows = num_rows;
		out.m_cols = num_rows;
		out.values = std::vector<T>(num_rows, T(1.0));
		out.col_indices = std::vector<int>(num_rows);
		std::iota(out.col_indices.begin(), out.col_indices.end(), 0);
		out.row_ptrs = std::vector<int>(num_rows+1);
		std::iota(out.row_ptrs.begin(), out.row_ptrs.end(), 0);

		return out;
	}

	SparseMatrix() : m_rows(0), m_cols(0) {};

	// Creates a square matrix from the input values
	SparseMatrix(std::vector<T>& vals, std::vector<int>& col_inds, std::vector<int>& row_ps)
	{
		assert(vals.size() == col_inds.size() && "The size of values passed must match the size of column IDs");
		m_rows = row_ps.size() - 1;
		m_cols = m_rows;
		row_ptrs = row_ps;
		values = vals;
		col_indices = col_inds;
	}

	// Creates a square matrix from the input values
	SparseMatrix(std::vector<T>& vals, std::vector<int>& col_inds, std::vector<int>& row_ps, int n_rows, int n_cols)
	{
		assert(vals.size() == col_inds.size() && "The size of values passed must match the size of column IDs");
		assert(n_rows == row_ps.size() - 1 && "The number of rows must be one less than the size of the row pointer vector");
		m_rows = n_rows;
		m_cols = n_cols;
		row_ptrs = row_ps;
		values = vals;
		col_indices = col_inds;
	}

	// Create an empty NxN SparseMatrix
	SparseMatrix(int n_rows)
	{
		m_rows = n_rows;
		m_cols = n_rows;
		row_ptrs = std::vector<int>(n_rows + 1, 0);
		values = std::vector<T>();
		col_indices = std::vector<int>();
	}

	// Create an empty NxM SparseMatrix
	SparseMatrix(int n_rows, int n_cols)
	{
		m_rows = n_rows;
		m_cols = n_cols;
		row_ptrs = std::vector<int>(n_rows + 1, 0);
		values = std::vector<T>();
		col_indices = std::vector<int>();
	}

	~SparseMatrix() {};

	friend std::ostream& operator <<(std::ostream& out, const SparseMatrix& mtx)
	{
		out << "\nSparseMatrix " << mtx.m_rows << "x" << mtx.m_cols << "\n";
		for (int r = 0; r < mtx.m_rows; ++r)
		{
			for (int col_start = mtx.row_ptrs[r]; col_start < mtx.row_ptrs[r + 1]; ++col_start)
			{
				out << "(" << r << "," << mtx.col_indices[col_start] << "): " << mtx.values[col_start] << "\n";
			}
		}
		out << "\n";
		return out;
	}

	Vec<T> operator*(const Vec<T>& x) const {
		assert(x.size() == static_cast<size_t>(m_cols) && "The input vector must have the same number of elements as there are columns");
		Vec<T> result(m_rows);
		for (int r = 0; r < m_rows; ++r)
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

	void operator*=(T scalar) {
		for (int i = 0; i < values.size(); ++i)
		{
			values[i] *= scalar;
		}
	}
	int rows() const {
		return m_rows;
	}
	int cols() const {
		return m_cols;
	}
	const std::vector<T>* getValues() const {
		return &values;
	}
	const std::vector<int>* getColIndices() const {
		return &col_indices;
	}
	const std::vector<int>* getRowPtrs() const {
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
		std::vector<int> row_ps(m_rows + 1, 0);
		row_ps[row_ps.size() - 1] = triplets.size();
		int curr_row = 0;
		for (int i=0; i < triplets.size(); ++i)
		{
			vals[i] = triplets[i].value;
			columns[i] = triplets[i].col;
			// If we switched rows, I need to store in the row_ptrs
			if (curr_row != triplets[i].row)
			{
				if (triplets[i].row > m_rows - 1)
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
		assert(x.size() == static_cast<size_t>(m_cols) && "The input vector must have the same number of elements as there are columns");
		if (result.size() != m_rows) result.resize(m_rows);

		for (int r = 0; r < m_rows; ++r)
		{
			T sum = T(0);
			for (int col_start = row_ptrs[r]; col_start < row_ptrs[r + 1]; ++col_start)
			{
				sum += values[col_start] * x[col_indices[col_start]];
			}
			result[r] = sum;
		}
	}

	void setZero() {
		std::fill(values.begin(), values.end(), T(0));
	}

	bool findElement(int row, int col, int& valIdx) const {
		assert(row <= m_rows - 1 && row >= 0 && "The row is outside of the matrix dimensions");
		assert(col <= m_cols - 1 && col >= 0 && "The column is outside of the matrix dimensions");
		// Invalid index is -1 here.
		valIdx = -1;
		int row_p = row_ptrs[row];
		if (row_ptrs[row + 1] - row_p == 0) {
			return false;
		}
		auto it = std::lower_bound(col_indices.begin() + row_p, col_indices.begin() + row_ptrs[row + 1], col);
		if (it != col_indices.begin() + row_ptrs[row + 1] && *it == col) {
			valIdx = (it - col_indices.begin());
			return true;
		}
		return false;
	}

	T operator()(int row, int col) const {
		int valIdx = -1;
		bool found = findElement(row, col, valIdx);
		if (!found) return T(0);
		return values[valIdx];
	}

	void setElement(int row, int col, T value) {
		int valIdx = -1;
		bool found = findElement(row, col, valIdx);
		if (!found){
			throw std::runtime_error("The element being set is not in the SparseMatrix pattern");
		}
		values[valIdx] = value;
	}
	void setByValueIndex(int idx, T value){
		values[idx] = value;
	}
	T& getByValueIndex(int idx) {
		return values[idx];
	}
	const T& getByValueIndex(int idx) const {
		return values[idx];
	}
	T* data() {
		return values.data();
	}
	const T* data() const {
		return values.data();
	}
};

using DSparseMatrix = SparseMatrix<double>;
using FSparseMatrix = SparseMatrix<float>;

template <typename T>
class Matrix {
	int m_rows;
	int m_cols;
	// We are storing the data row-wise.
	std::vector<T> m_data;

public:
	
	Matrix(int r, int c, const std::vector<T>& data) : m_rows(r), m_cols(c), m_data(data)
	{
		assert(data.size() == r * c && "The input data must have dimensions rows*cols");
	}
	// initialize an empty NxM matrix
	Matrix(int r, int c) : m_rows(r), m_cols(c), m_data(m_rows * m_cols)
	{
		assert(r >= 0 && c >= 0 && "Cannot have rows or columns of size 0");
	}
	// Create a square NxN identity matrix
	static Matrix<T> Identity(int r)
	{
		Matrix<T> out(r, r);
		for (int i = 0; i < r; ++i)
		{
			out.m_data[i * r + i] = T(1);
		}
		return out;
	}

	Matrix<T> transpose() const
	{
		Matrix<T> out(m_cols, m_rows);

		for (int r = 0; r < m_rows; ++r)
		{
			for (int c = 0; c < m_cols; ++c)
			{
				out.m_data[c * m_rows + r] = m_data[r * m_cols + c];
			}
		}

		return out;
	}
	const T& operator()(int r, int c) const
	{
		assert(r >= 0 && r < m_rows && "The input row number is outside of row range");
		assert(c >= 0 && c < m_cols && "The input column number is outside of column range");
		return m_data[r * m_cols + c];
	}
	T& operator()(int r, int c)
	{
		assert(r >= 0 && r < m_rows && "The input row number is outside of row range");
		assert(c >= 0 && c < m_cols && "The input column number is outside of column range");
		return m_data[r * m_cols + c];
	}
	friend std::ostream& operator <<(std::ostream& out, const Matrix& mtx)
	{
		out << "\nMatrix " << mtx.m_rows << "x" << mtx.m_cols << "\n";
		out << "[";
		for (int r = 0; r < mtx.m_rows; ++r)
		{
			if (r == 0) out << "[";
			else out << " [";
			for (int c = 0; c < mtx.m_cols; ++c)
			{
				if (c < mtx.m_cols - 1)
				{
					out << mtx.m_data[r * mtx.m_cols + c] << ", ";
				}
				else
				{
					out << mtx.m_data[r * mtx.m_cols + c];
				}
			}
			out << "]";
			if (r != mtx.m_rows - 1) out << "\n";
		}
		out << "]\n";
		return out;
	}

	Matrix<T> operator*(const Matrix<T>& mtx) const
	{
		assert(mtx.m_rows == m_cols && "Dimensionality mismatch between matrices");
		Matrix<T> out(m_rows, mtx.m_cols);
		// We use the IKJ method
		const T* __restrict A_data = m_data.data();
		const T* __restrict B_data = mtx.m_data.data();
		T* __restrict C_data = out.m_data.data();
		// For each row in A
		for (int i = 0; i < m_rows; ++i)
		{
			// Jump to the row of A
			const T* __restrict A_row = A_data + i * m_cols;
			// Jump to the C row
			T* __restrict C_row = C_data + i * out.m_cols;
			// Now we need a row of B that will change with each column of A
			for (int k = 0; k < m_cols; ++k)
			{
				const T* __restrict B_row = B_data + k * mtx.m_cols;
				T val = A_row[k];
				if (val == T(0)) continue;
				for (int j = 0; j < mtx.m_cols; ++j)
				{
					C_row[j] += B_row[j] * val;
				}
			}
		}
		return out;
	}

	friend Matrix<T> operator*(T scalar, const Matrix<T>& mtx)
	{
		Matrix<T> out{ mtx.rows(), mtx.cols()};

		std::transform(mtx.m_data.begin(), mtx.m_data.end(), out.m_data.begin(),
			[scalar](T val) {return val * scalar; });

		return out;
	}

	friend Matrix<T> operator*(const Matrix<T>& mtx, T scalar)
	{
		return scalar * mtx;
	}
	
	void operator*=(const T& scalar)
	{
		for (int i = 0; i < m_data.size(); ++i)
		{
			m_data[i] *= scalar;
		}
	}

	Matrix<T> operator+(const Matrix<T>& mtx) const
	{
		assert(mtx.m_rows == m_rows && mtx.m_cols == m_cols && "Dimensionality mismatch between matrices");
		Matrix<T> out{m_rows, m_cols};
		for (int i = 0; i < out.m_data.size(); ++i)
		{
			out.m_data[i] = m_data[i] + mtx.m_data[i];
		}
		return out;
	}

	Matrix<T> operator-(const Matrix<T>& mtx) const
	{
		assert(mtx.m_rows == m_rows && mtx.m_cols == m_cols && "Dimensionality mismatch between matrices");
		Matrix<T> out{ m_rows, m_cols };
		for (int i = 0; i < out.m_data.size(); ++i)
		{
			out.m_data[i] = m_data[i] - mtx.m_data[i];
		}
		return out;
	}

	bool operator==(const Matrix<T>& mtx) const
	{
		if (mtx.rows() != m_rows || mtx.cols() != m_cols) return false;
		for (int i = 0; i < m_data.size(); ++i)
		{
			if (m_data[i] != mtx.m_data[i]) return false;
		}
		return true;
	}

	void setZero()
	{
		std::fill(m_data.begin(), m_data.end(), T(0));
	}
	int rows() const {
		return m_rows;
	}
	int cols() const {
		return m_cols;
	}
	T* data() {
		return m_data.data();
	}
	const T* data() const {
		return m_data.data();
	}

};

using FMatrix = Matrix<float>;
using DMatrix = Matrix<double>;

template <typename T>
class Matrix3 {
	T m_data[9];

public:

	Matrix3() : m_data{ 0 } {
	}
	// initialize with all elements as a value
	Matrix3(T value)
	{
		std::fill(m_data, m_data + 9, value);
	}
	// Create a square NxN identity matrix
	static Matrix3<T> Identity()
	{
		Matrix3<T> out{};
		out.m_data[0] = T(1);
		out.m_data[4] = T(1);
		out.m_data[8] = T(1);
		return out;
	}

	Matrix3<T> transpose() const
	{
		Matrix3<T> out{};

		for (int r = 0; r < 3; ++r)
		{
			for (int c = 0; c < 3; ++c)
			{
				out.m_data[c * 3 + r] = m_data[r * 3 + c];
			}
		}

		return out;
	}
	const T& operator()(int r, int c) const
	{
		assert(r >= 0 && r < 3 && "The input row number is outside of row range");
		assert(c >= 0 && c < 3 && "The input column number is outside of column range");
		return m_data[r * 3 + c];
	}
	T& operator()(int r, int c)
	{
		assert(r >= 0 && r < 3 && "The input row number is outside of row range");
		assert(c >= 0 && c < 3 && "The input column number is outside of column range");
		return m_data[r * 3 + c];
	}
	friend std::ostream& operator <<(std::ostream& out, const Matrix3& mtx)
	{
		out << "\nMatrix " << 3 << "x" << 3 << "\n";
		out << "[";
		for (int r = 0; r < 3; ++r)
		{
			if (r == 0) out << "[";
			else out << " [";
			for (int c = 0; c < 3; ++c)
			{
				if (c < 2)
				{
					out << mtx.m_data[r * 3 + c] << ", ";
				}
				else
				{
					out << mtx.m_data[r * 3 + c];
				}
			}
			out << "]";
			if (r != 3 - 1) out << "\n";
		}
		out << "]\n";
		return out;
	}

	Matrix3<T> operator*(const Matrix3<T>& mtx) const
	{
		Matrix3<T> out{};
		// Unroll the whole multiplication here just to speed the hell out of it
		// Out Row 1
		out.m_data[0] = m_data[0] * mtx.m_data[0] + m_data[1] * mtx.m_data[3] + m_data[2] * mtx.m_data[6];
		out.m_data[1] = m_data[0] * mtx.m_data[1] + m_data[1] * mtx.m_data[4] + m_data[2] * mtx.m_data[7];
		out.m_data[2] = m_data[0] * mtx.m_data[2] + m_data[1] * mtx.m_data[5] + m_data[2] * mtx.m_data[8];
		// Out Row 2
		out.m_data[3] = m_data[3] * mtx.m_data[0] + m_data[4] * mtx.m_data[3] + m_data[5] * mtx.m_data[6];
		out.m_data[4] = m_data[3] * mtx.m_data[1] + m_data[4] * mtx.m_data[4] + m_data[5] * mtx.m_data[7];
		out.m_data[5] = m_data[3] * mtx.m_data[2] + m_data[4] * mtx.m_data[5] + m_data[5] * mtx.m_data[8];
		// Out Row 3
		out.m_data[6] = m_data[6] * mtx.m_data[0] + m_data[7] * mtx.m_data[3] + m_data[8] * mtx.m_data[6];
		out.m_data[7] = m_data[6] * mtx.m_data[1] + m_data[7] * mtx.m_data[4] + m_data[8] * mtx.m_data[7];
		out.m_data[8] = m_data[6] * mtx.m_data[2] + m_data[7] * mtx.m_data[5] + m_data[8] * mtx.m_data[8];
		return out;
	}

	friend Matrix3<T> operator*(T scalar, const Matrix3<T>& mtx)
	{
		Matrix3<T> out{};

		for (int i = 0; i < 9; ++i)
		{
			out.m_data[i] = mtx.m_data[i] * scalar;
		}

		return out;
	}

	friend Matrix3<T> operator*(const Matrix3<T>& mtx, T scalar)
	{
		return scalar * mtx;
	}

	void operator*=(const T& scalar)
	{
		for (int i = 0; i < 9; ++i)
		{
			m_data[i] *= scalar;
		}
	}

	Matrix3<T> operator+(const Matrix3<T>& mtx) const
	{
		Matrix3<T> out{};
		for (int i = 0; i < 9; ++i)
		{
			out.m_data[i] = m_data[i] + mtx.m_data[i];
		}
		return out;
	}

	void operator+=(const Matrix3<T>& mtx)
	{
		for (int i = 0; i < 9; ++i)
		{
			m_data[i] += mtx.m_data[i];
		}
	}

	Matrix3<T> operator-(const Matrix3<T>& mtx) const
	{
		Matrix3<T> out{};
		for (int i = 0; i < 9; ++i)
		{
			out.m_data[i] = m_data[i] - mtx.m_data[i];
		}
		return out;
	}

	void operator-=(const Matrix3<T>& mtx)
	{
		for (int i = 0; i < 9; ++i)
		{
			m_data[i] -= mtx.m_data[i];
		}
	}

	bool operator==(const Matrix3<T>& mtx) const
	{
		for (int i = 0; i < 9; ++i)
		{
			if (m_data[i] != mtx.m_data[i]) return false;
		}
		return true;
	}
	void setZero()
	{
		std::fill(m_data, m_data + 9, T(0));
	}
	int rows() const {
		return 3;
	}
	int cols() const {
		return 3;
	}
	T* data() {
		return m_data;
	}
	const T* data() const {
		return m_data;
	}

};

using FMatrix3 = Matrix3<float>;
using DMatrix3 = Matrix3<double>;

template <typename T>
// Need to define outer AFTER we have the Matrix<T>.data() defined
Matrix3<T> Vec3<T>::outer(const Vec3<T>& vec) const {
	Matrix3<T> out{};
	T* __restrict out_data = out.data();
	out_data[0] = m_data[0] * vec.m_data[0];
	out_data[1] = m_data[0] * vec.m_data[1];
	out_data[2] = m_data[0] * vec.m_data[2];
	out_data[3] = m_data[1] * vec.m_data[0];
	out_data[4] = m_data[1] * vec.m_data[1];
	out_data[5] = m_data[1] * vec.m_data[2];
	out_data[6] = m_data[2] * vec.m_data[0];
	out_data[7] = m_data[2] * vec.m_data[1];
	out_data[8] = m_data[2] * vec.m_data[2];
	return out;
}

template <typename T>
int SolveCG(const SparseMatrix<T>& A, const Vec<T>& b, Vec<T>& x, int max_iter = 100, T tol = T(1e-8))
{
	Vec<T>Ap, r, p;
	
	r = b - A * x;
	T r0lensq = r.lensq();
	p = r;

	int curr_iter = 0;

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