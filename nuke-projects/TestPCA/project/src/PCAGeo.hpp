#pragma once

#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/Knobs.h"
#include "DDImage/ViewFrustum.h"

#include <cassert>

using namespace DD::Image;

static const char* const CLASS = "PCAGeo";
static const char* const HELP = "Combine geometries of two objects";

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

class PCAGeo : public GeoOp
{
private:
	const static int N = 100; // max number of inputs
	int N_pca; // only first N_pca ordered PCA components will be shown
	bool pretty_show; // should position_delta be applied to all the models for a good exposure
	float var_threshold; // only PCA with cumulative variance proportion >= threshold will be loaded
	float d_x; // delta to be used in pretty show
	
protected:
	void _validate(bool for_real) override;

public:
	static const Description description;
	const char* Class() const override;
	const char* node_help() const override;

	PCAGeo(Node* node);

	int minimum_inputs() const override;
	int maximum_inputs() const override;

	void get_geometry_hash() override;

	void geometry_engine(Scene& scene, GeometryList& out) override;

	void knobs(Knob_Callback f) override;
};
