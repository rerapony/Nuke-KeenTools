#include "CombineMultGeometry.hpp"

void CombineMultGeometry::_validate(bool for_real)
{
	Op* this_op = Op::input(0);

	this_op->validate(for_real);
	
	for (int i = 1; i < N+1; i++)
	{
		if (Op::input(i) != nullptr)
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
	return 1;
}

int CombineMultGeometry::maximum_inputs() const 
{
	return N+1;
}

void CombineMultGeometry::get_geometry_hash()
{
	GeoOp::get_geometry_hash();

	geo_hash[Group_Points].append(_param, N);
}

void CombineMultGeometry::geometry_engine(Scene& scene, GeometryList& out)
{
	input0()->get_geometry(scene, out);

	unsigned int objs = out.objects();
	
	for (int geo_id = 1; geo_id < N+1; ++geo_id)
	{
		if (Op::input(geo_id) == nullptr)
			break;
		
		Scene other_scene;
		GeometryList other;
		input(geo_id)->get_geometry(other_scene, other);

		unsigned int objs = out.objects();
		unsigned int other_objs = other.objects();
		assert(objs == other_objs);

		for (unsigned int i = 0; i < objs; ++i)
		{
			PointList* points = out.writable_points(i);
			GeoInfo& other_info = other[i];
			const PointList* other_points = other_info.point_list();

			const unsigned n = points->size();
			const unsigned other_n = other_points->size();

			assert(n == other_n);

			for (unsigned j = 0; j < n; j++) {
				Vector3& v = (*points)[j];
				const Vector3& other_v = (*other_points)[j];
				v.x += _param[geo_id] * (other_v.x - v.x);
				v.y += _param[geo_id] * (other_v.y - v.y);
				v.z += _param[geo_id] * (other_v.z - v.z);
			}
		}
	}
}

void CombineMultGeometry::knobs(Knob_Callback f)
{
	MultiFloat_knob(f, _param, N,"combination param", "combination param");
	SetRange(f, 0, 1);
}

static Op* build(Node* node)
{
	return new CombineMultGeometry(node);
}

const Op::Description CombineMultGeometry::description(CLASS, build);