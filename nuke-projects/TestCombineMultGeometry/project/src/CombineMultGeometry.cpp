#include "CombineMultGeometry.hpp"
#include <vector>

void CombineMultGeometry::_validate(bool for_real)
{
	Op* this_op = Op::input(0);
	Op* other_op = Op::input(1);

	this_op->validate(for_real);
	other_op->validate(for_real);
	
	for (int i = 2; i < N; i++)
	{
		if (Op::input(i))
		{
			Op* temp = Op::input(i);
			temp->validate(for_real);
		}
	}
	
	
	GeoOp::_validate(for_real);
}

const char* CombineMultGeometry::Class() const
{
	return CLASS;
}
const char* CombineMultGeometry::node_help() const
{
	return HELP;
}

CombineMultGeometry::CombineMultGeometry(Node* node) : GeoOp(node) {
	for (int i = 0; i < N; ++i)
	{
		_param[i] = 0.5f;
	}
}

int CombineMultGeometry::minimum_inputs() const 
{
	return 2;
}

int CombineMultGeometry::maximum_inputs() const 
{
	return N;
}

void CombineMultGeometry::get_geometry_hash()
{
	GeoOp::get_geometry_hash();
}

void CombineMultGeometry::geometry_engine(Scene& scene, GeometryList& out)
{
	input0()->get_geometry(scene, out);
	// processing first input as base case
	unsigned int objs = out.objects();
	for (unsigned int j = 0; j < objs; ++j)
	{
		PointList* points = out.writable_points(j);
		const unsigned n = points->size();

		for (unsigned k = 0; k < n; k++) {
			Vector3& v = (*points)[k];
			v.x *= _param[0];
			v.y *= _param[0];
			v.z *= _param[0];
		}
	}

	
	if (Op::input(1) == nullptr)
		error("Can't work with one geometry.");
	
	for (int i = 1; i < N; ++i)
	{
		// ran out of valid inputs
		if (Op::input(i) == nullptr)
			break;

		Scene other_scene;
		GeometryList other;
		input(i)->get_geometry(other_scene, other);
		
		unsigned int other_objs = other.objects();
		assert(objs == other_objs);

		for (unsigned int j = 0; j < objs; ++j)
		{
			PointList* points = out.writable_points(j);
			GeoInfo& other_info = other[j];
			const PointList* other_points = other_info.point_list();

			const unsigned n = points->size();
			const unsigned other_n = other_points->size();
			assert(n == other_n);

			for (unsigned k = 0; k < n; k++) {
				Vector3& v = (*points)[k];
				const Vector3& other_v = (*other_points)[k];
				v.x += _param[i] * other_v.x;
				v.y += _param[i] * other_v.y;
				v.z += _param[i] * other_v.z;
			}
		}
	}
}

void CombineMultGeometry::knobs(Knob_Callback f)
{
	MultiFloat_knob(f, _param, N,"combination param", "combination param");
	SetRange(f, 0, 2);
}

static Op* build(Node* node)
{
	return new CombineMultGeometry(node);
}

const Op::Description CombineMultGeometry::description(CLASS, build);