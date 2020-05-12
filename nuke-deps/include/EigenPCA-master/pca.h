#ifndef PCA_H_
#define PCA_H_

#include <vector>
#include "D:/Work/nuke-practice/nuke-deps/include/eigen-3.3.7/Eigen/Dense"

class Pca {
private:
	std::vector<float>  _x;   // Initial matrix as vector filled by rows.
	Eigen::MatrixXf     _xXf; // Initial matrix as Eigen MatrixXf structure
	unsigned int  _nrows,     // Number of rows in matrix x.
		_ncols;     // Number of cols in matrix x.


	std::vector<std::vector<float>> pca_vecs;
	std::vector<float> mean_vec;
	std::vector<float> pca_vars;
	std::vector<float> var_props;
	int pca_rows;
	int pca_cols;

public:
	//! Initializing values and performing PCA
	/*!
	The main method for performin Principal Component Analysis
	\param  x     Initial data matrix
	\param  nrows Number of matrix rows
	\param  ncols Number of matrix cols
	\result
	0 if everything is Ok
	-1 if there were some errors
	*/
	int Calculate(std::vector<float>& x, const unsigned int& nrows, const unsigned int& ncols);
	//! Return number of rows in initial matrix
	/*!
	\result Number of rows in initial matrix
	*/
	unsigned int nrows(void);
	//! Return number of cols in initial matrix
	/*!
	\result Number of cols in initial matrix
	*/
	unsigned int ncols(void);

	//! return pca components
	std::vector<std::vector<float>> pca_components();

	//! return pca variance (magnitude)
	std::vector<float> pca_variance();

	//! return mean vector
	std::vector<float> mean();

	//! return pca components size
	std::pair<int, int> pca_size();

	//! return proportions of variances
	std::vector<float> var_proportions();
	
	//! Class constructor
	Pca(void);
	//! Class destructor
	~Pca(void);
};

#endif