#include <cassert>
#include <iostream>
#include <vector>
#include "D:\Work\nuke-practice\nuke-deps\include\EigenPCA-master\pca.cpp"

using namespace std;

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
{
	assert(a.size() == b.size());

	std::vector<T> result(a.size());

	for (int i = 0; i < a.size(); i++)
	{
		result[i] = a[i] + b[i];
	}
	
	return result;
}

template <typename T>
std::vector<T> operator*(const float& a, const std::vector<T>& b)
{

	std::vector<T> result(b.size());

	for (int i = 0; i < b.size(); i++)
	{
		result[i] = a*b[i];
	}

	return result;
}

int main()
{
	// (1;4)(10;40)(5;20)(0;0)(14;56)(8;32)(6;24)(2;8)(20;80)(13;52)

	float arr[] = { 4.79, -1.38, 5.04, 5.83, 7.21, 8.37, 6.39, 4.17, 5.35, 5.05, 2.57, 4.17, 5.85, 5.08, 3.96,
		2.77, 5.96, 6.3, 1.1, 3.17, 4.82, 4.94, 7.91, 6.57, 4.22, 1.17, 6.44, 1.65, 3.59, 6.76,
		4.64, 1.95, 2.03, 4.32, 6.76, 3.56, 1.85, 3.64, 3.71, 3.12, 7.74, 6.4, 7.52, -1.26, 1.52,
		5.17, 2.61, 6.85, 4.8, 3.28, 4.84, 3.74, 4.02, 0.74, 5.04, 4.62, 3.45, 6.09, 7.31, 3.44,
		4.8, 5.88, 2.97, 5.92, 5.12, 2.51, 4.45, 1.23, 4.61, 5.52, 6.01, 3.27, 3.7, 8.21, 7.4,
		4.05, 8.88, 4.77, 3.93, 6.31, 6.05, 4.85, 5.16, 6.45, 2.48, 5.38, 3.43, 5.46, 6.43, 6.26,
		5.27, 7.21, 2.99, 4.79, 1.44, 3.55, 5.37, 4.21, 6.34, 4.41, 3.06, 5.29, 7.44, 0.62, 5.06,
		3.92, 4.56, 4.98, 2.17, 5.2, 4.58, 4.86, 6.17, 6.8, 4.7, 3.7, 3.83, 4.77, 6.26, 6.0 };
	
	vector<float> v(std::begin(arr), std::end(arr));

	Pca *pca = new Pca();
	int init_result = pca->Calculate(v, 40, 3);
	assert(init_result == 0);


	cout << "Pca components" << endl;
	for (auto& vec : pca->pca_components())
	{
		for (auto& var : vec)
		{
			cout << var << " ";
		}
		cout << endl;
	}
	cout << endl;

	cout << "Mean" << endl;
	for (auto& vec : pca->mean())
	{
		cout << vec << " ";
	}
	cout << endl;

	cout << "Variance proportions" << endl;
	for (auto& val : pca->var_proportions())
	{
		cout << val << " ";
	}
	cout << endl;
	
	cout << "Variances" << endl;
	for (auto& val : pca->pca_variance())
	{
		cout << val << " ";
	}
	cout << endl;
	std::pair<float, float> pca_size = pca->pca_size();
	
	cout << "Result" << endl;
	for (int i = 0; i < pca_size.first; i++)
	{
		auto extreme_point = (2 * sqrt(pca->pca_variance()[i]))*pca->pca_components()[i];
		auto res = pca->mean() + extreme_point;
		for (auto& var : res)
		{
			cout << var << " ";
		}
		cout << endl;
	}
	cout << endl;
	
	return 0;
}