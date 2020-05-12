#define NODEBUG
#include <iostream>
#ifdef DEBUG
#include <iterator>
#endif
#include "D:/Work/nuke-practice/nuke-deps/include/eigen-3.3.7/Eigen/SVD"
#include "pca.h"

using namespace std;
using namespace Eigen;

int Pca::Calculate(vector<float> &x,
	const unsigned int &nrows,
	const unsigned int &ncols) {
	_ncols = ncols;
	_nrows = nrows;
	if (x.size() != _nrows*_ncols) {
		return -1;
	}
	if ((1 == _ncols) || (1 == nrows)) {
		return -1;
	}
	// Convert vector to Eigen 2-dimensional matrix
	//Map<MatrixXf> _xXf(x.data(), _nrows, _ncols);
	_xXf.resize(_nrows, _ncols);
	for (unsigned int i = 0; i < _nrows; ++i) {
		for (unsigned int j = 0; j < _ncols; ++j) {
			_xXf(i, j) = x[j + i*_ncols];
		}
	}
	// Mean and standard deviation for each column
	VectorXf mean_vector = _xXf.colwise().mean();
#ifdef DEBUG
	cout << "Matrix:" << endl << _xXf << endl;
	cout << "Mean:" << endl << mean_vector << endl;
#endif

	for (int i = 0; i < _ncols; i++)
	{
		mean_vec.push_back(mean_vector(i, 0));
	}

	for (unsigned int i = 0; i < _ncols; ++i) {
		_xXf.col(i) -= VectorXf::Constant(_nrows, mean_vector(i));
	}
	float denom = static_cast<float>((_nrows > 1) ? _nrows - 1 : 1);

	JacobiSVD<MatrixXf> svd(_xXf, ComputeThinV);
	VectorXf eigen_singular_values = svd.singularValues();
	VectorXf eigen_values = eigen_singular_values.array().square();
	
	for (unsigned int i = 0; i < eigen_values.size(); ++i) {
		pca_vars.push_back(eigen_values(i)/denom);
	}

	MatrixXf eigen_vectors = svd.matrixV();

#ifdef DEBUG
	cout << "SVD values: " << endl << eigen_values << endl;
	cout << "Eigen vectors:" << endl << svd.matrixV() << endl;
#endif
	
	auto eig_rows = eigen_vectors.rows();
	auto eig_cols = eigen_vectors.cols();

	pca_rows = eig_cols;
	pca_cols = eig_rows;
	
	for (int i = 0; i < eig_cols; i++)
	{
		std::vector<float> pca_row;
		for (int j = 0; j < eig_rows; j++)
		{
			pca_row.push_back(eigen_vectors(j, i));
		}
		pca_vecs.push_back(pca_row);
	}
	
	return 0;
}

std::vector<std::vector<float>> Pca::pca_components()
{
	return pca_vecs;
}

std::vector<float> Pca::pca_variance()
{
	return pca_vars;
}

std::vector<float> Pca::mean()
{
	return mean_vec;
}

std::pair<int, int> Pca::pca_size()
{
	return std::make_pair(pca_rows, pca_cols);
}

Pca::Pca(void) {
	_nrows = 0;
	_ncols = 0;
}
Pca::~Pca(void) {
	_xXf.resize(0, 0);
	_x.clear();
}
