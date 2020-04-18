#include "CombineGeometry.hpp"

void CombineGeometry::_validate(bool for_real)
{
	Op* this_op = Op::input(0);
	Op* other_op = Op::input(1);

	this_op->validate(for_real);
	other_op->validate(for_real);

	GeoOp::_validate(for_real);
}

const char* CombineGeometry::Class() const
{
	return CLASS;
}
const char* CombineGeometry::node_help() const
{
	return HELP;
}

CombineGeometry::CombineGeometry(Node* node) : GeoOp(node) {
	_param = 0.5f;
}

int CombineGeometry::minimum_inputs() const 
{
	return 2;
}

int CombineGeometry::maximum_inputs() const 
{
	return 2;
}

void CombineGeometry::get_geometry_hash()
{
	GeoOp::get_geometry_hash();
}

void CombineGeometry::geometry_engine(Scene& scene, GeometryList& out)
{
	input0()->get_geometry(scene, out);

	if (Op::input(1) == nullptr)
		error("Can't work with one geometry.");

	Scene other_scene;
	GeometryList other;
	input1()->get_geometry(other_scene, other);

	unsigned int objs = out.objects();
	unsigned int other_objs = other.objects();
	assert(objs == other_objs);

	std::cout << objs;

	for (unsigned int i = 0; i < objs; ++i)
	{
		PointList* points = out.writable_points(i);
		GeoInfo& other_info = other[i];
		const PointList* other_points = other_info.point_list();

		const unsigned n = points->size();
		const unsigned other_n = other_points->size();

		std::cout << n;

		assert(n == other_n);

		for (unsigned j = 0; j < n; j++) {
			Vector3& v = (*points)[j];
			const Vector3& other_v = (*other_points)[j];
			v.x = _param * (v.x + other_v.x);
			v.y = _param * (v.y + other_v.y);
			v.z = _param * (v.z + other_v.z);
		}
	}
}

void CombineGeometry::knobs(Knob_Callback f)
{
	Float_knob(f, &_param, "combination param", "combination param");
	SetRange(f, 0, 2);
}

static Op* build(Node* node)
{
	return new CombineGeometry(node);
}

const Op::Description CombineGeometry::description(CLASS, build);