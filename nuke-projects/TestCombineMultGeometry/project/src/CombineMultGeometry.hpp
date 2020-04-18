#pragma once

#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/Knobs.h"
#include "DDImage/ViewFrustum.h"

#include <cassert>

using namespace DD::Image;

static const char* const CLASS = "CombineMultGeometry";
static const char* const HELP = "Combine geometries of two objects";


class CombineMultGeometry : public GeoOp
{
	static const int N = 10;
	float _param[N];
	
protected:
	void _validate(bool for_real) override;

public:
	static const Description description;
	const char* Class() const override;
	const char* node_help() const override;

	CombineMultGeometry(Node* node);

	int minimum_inputs() const override;
	int maximum_inputs() const override;

	void get_geometry_hash() override;

	void geometry_engine(Scene& scene, GeometryList& out) override;

	void knobs(Knob_Callback f) override;
};
