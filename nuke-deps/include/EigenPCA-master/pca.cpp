#define NODEBUG
#include <iostream>
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
	MatrixXf cov;

	// вот тут проблемы
	cov = (1.0 / (_nrows-1)) * _xXf.adjoint() * _xXf;

#ifdef DEBUG
	cout << "Covariance matrix:" << endl << cov << endl;
#endif
	
	EigenSolver<MatrixXf> solver(cov);

#ifdef DEBUG
	cout << "Eigenvectors:" << endl << solver.eigenvectors() << endl;
	cout << "Eigenvalues:" << endl << solver.eigenvalues() << endl;
#endif

	auto eigen_vecs = solver.eigenvectors();
	auto eigen_vals = solver.eigenvalues();
	
	for (int i = 0; i < _ncols; i++)
	{
		std::vector<float> pca_row;
		for (int j = 0; j < _ncols; j++)
		{
			pca_row.push_back(eigen_vecs(j, i).real());
		}
		pca_vecs.push_back(pca_row);
	}

	pca_rows = _ncols;
	pca_cols = _ncols;

	for (int i = 0; i < _ncols; i++)
	{
		pca_vars.push_back(eigen_vals(i, 0).real());
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
  _nrows      = 0;
  _ncols      = 0;
}
Pca::~Pca(void) { 
  _xXf.resize(0, 0);
  _x.clear();
}
