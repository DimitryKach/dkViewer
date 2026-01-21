#include <vector>
#include <memory>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <cmath>


class DVec {
	std::vector<double> m_data;
public:
	explicit DVec(std::initializer_list<double> list) : m_data(list) {};
	explicit DVec(const std::vector<double>& _data) : m_data(_data) {};
	explicit DVec(size_t N) {
		m_data = std::vector<double>(N, 0.0);
	};
	DVec() {};
	~DVec() {};

	bool operator==(const DVec& a) {
		bool matching = a.m_data.size() == m_data.size();
		if (!matching) return matching;
		for (int i = 0; i < m_data.size(); ++i)
		{
			matching = std::abs(m_data[i] - a.m_data[i]) < 1e-9;
			if (!matching) return matching;
		}
		return matching;
	}

	DVec operator+(const DVec& a) const {
		assert(a.m_data.size() == m_data.size() && "The vector you are adding must match in dimensions");
		DVec out(m_data);
		for (int i = 0; i < m_data.size(); ++i)
		{
			out[i] += a[i];
		}
		return out;
	}

	void operator+=(const DVec& a) {
		assert(a.m_data.size() == m_data.size() && "The vector you are adding must match in dimensions");
		for (int i = 0; i < m_data.size(); ++i)
		{
			m_data[i] += a.m_data[i];
		}
	}

	DVec operator-(const DVec& a) const {
		assert(a.m_data.size() == m_data.size() && "The vector you are subtracting must match in dimensions");
		DVec out(m_data);
		for (int i = 0; i < m_data.size(); ++i)
		{
			out[i] -= a[i];
		}
		return out;
	}

	friend DVec operator*(double scalar, const DVec& vec) {
		DVec out{ vec.m_data };
		for (int i = 0; i < vec.m_data.size(); ++i)
		{
			out.m_data[i] *= scalar;
		}
		return out;
	}

	friend DVec operator*(const DVec& vec, double scalar) {
		return scalar * vec;
	}

	void operator-=(const DVec& a) {
		assert(a.m_data.size() == m_data.size() && "The vector you are subtracting must match in dimensions");
		for (int i = 0; i < m_data.size(); ++i)
		{
			m_data[i] -= a.m_data[i];
		}
	}

	// Note to self - we needed an & because we are returning an lvalue that is a reference instead of a copy.
	double& operator[](size_t i) {
		return m_data[i];
	}
	// need a const accessor when the DVec variable is declared const
	const double& operator[](size_t i) const {
		return m_data[i];
	}

	double dot(const DVec& a) {
		assert(a.m_data.size() == m_data.size() && "The vectors for the inner product must match in dimensions");
		double out = 0.0;
		for (int i = 0; i < m_data.size(); ++i)
		{
			out += m_data[i] * a.m_data[i];
		}
		return out;
	}

	size_t size() const {
		return m_data.size();
	}

	friend std::ostream& operator <<(std::ostream& out, const DVec& vec)
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

	double length() {
		return std::sqrt(lensq());
	}

	double lensq(){
		return this->dot(*this);
	}

};

class SparseMatrix {
	int rows;
	// Compressed Sparse Row format
	std::vector<double> values;
	std::vector<int> col_indices;
	std::vector<int> row_ptrs; // Size is rows + 1

	// TODO: Implement Matrix-Vector Multiply

	friend std::ostream & operator <<(std::ostream& out, const SparseMatrix& mtx)
	{
		out << "\nSparseMatrx " << mtx.rows << "x" << mtx.rows << "\n";
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
	SparseMatrix(std::vector<double>& vals, std::vector<int>& col_inds, std::vector<int>& row_ps)
	{
		assert(vals.size() == col_inds.size() && "The values passed must match the column IDs");
		rows = row_ps.size() - 1;
		row_ptrs = row_ps;
		values = vals;
		col_indices = col_inds;
	}
	~SparseMatrix() {};
	DVec operator*(const DVec& x) const {
		assert(x.size() == static_cast<size_t>(rows) && "The input vector must have the same number of elements as there are rows");
		DVec result(rows);
		for (int r = 0; r < rows; ++r)
		{
			// Suggested accumulator living on register mem
			// TODO: Verify the performance gain here if possible
			double sum = 0.0;
			for (int col_start = row_ptrs[r]; col_start < row_ptrs[r + 1]; ++col_start)
			{
				sum += values[col_start] * x[col_indices[col_start]];
			}
			result[r] = sum;
		}
		return result;
	}
};

int SolveCG(const SparseMatrix& A, const DVec& b, DVec& x, int max_iter = 100, double tol = 1e-6)
{
	DVec r0 = b - A * x;
	double r0lensq = r0.lensq();
	DVec p0 = r0;

	double k = 0.0;

	int curr_iter = 0;

	while (curr_iter < max_iter && r0lensq > tol*tol)
	{
		DVec Ap = A * p0;
		double alpha = r0lensq / p0.dot(Ap);
		x = x + alpha * p0;
		DVec r1 = r0 - alpha * (Ap);
		double r1lensq = r1.lensq();
		if (r1lensq < tol * tol) break;
		double beta = r1lensq / r0lensq;
		DVec p1 = r1 + beta * p0;
		p0 = p1;
		r0 = r1;
		r0lensq = r1lensq;
		++curr_iter;
	}
	
	return curr_iter;
}