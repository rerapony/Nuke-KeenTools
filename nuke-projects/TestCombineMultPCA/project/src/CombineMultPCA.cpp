#include "CombineMultPCA.hpp"

#include <vector>

void CombineMultPCA::_validate(bool for_real)
{
	Op* this_op = Op::input(0);
	this_op->validate(for_real);
	GeoOp::_validate(for_real);
}

const char* CombineMultPCA::Class() const
{
	return CLASS;
}
const char* CombineMultPCA::node_help() const
{
	return HELP;
}

CombineMultPCA::CombineMultPCA(Node* node) : GeoOp(node) {
	for (int i = 0; i < N; ++i)
	{
		_param[i] = 0.5f;
	}
}

int CombineMultPCA::minimum_inputs() const
{
	return 1;
}

int CombineMultPCA::maximum_inputs() const 
{
	return 1;
}

void CombineMultPCA::get_geometry_hash()
{
	GeoOp::get_geometry_hash();

	geo_hash[Group_Points].append(_param, N);
}

void CombineMultPCA::geometry_engine(Scene& scene, GeometryList& out)
{
	Scene other_scene;
	GeometryList other;
	
	input0()->get_geometry(other_scene, other);

	const unsigned int objs = other.objects();
	assert(objs <= N);
	
	GeoInfo& info = other[0];
	GeoInfo* info_to_copy = &other[0];
	const PointList* points = info.point_list();
	const unsigned int points_n = points->size();
	std::vector<Vector3> mean_points(points_n);
	
	// first object points
	for (unsigned int j = 0; j < points_n; j++) {
		const Vector3& v = (*points)[j];
		mean_points[j] = v;
	}
	
	for (unsigned int obj_id = 1; obj_id < objs; obj_id++) {
		GeoInfo& other_info = other[obj_id];
		const PointList* other_points = other_info.point_list();
		assert(points_n == other_points->size());

		for (unsigned j = 0; j < points_n; j++) {
			const Vector3& v = (*other_points)[j];
			mean_points[j] += v;
		}
	}
	// average points
	for (unsigned j = 0; j < points_n; j++) {
		mean_points[j] /= (float)objs;
	}
	
	for (unsigned int obj_id = 0; obj_id < objs; obj_id++) {
		GeoInfo& other_info = other[obj_id];
		const PointList* other_points = other_info.point_list();
		assert(points_n == other_points->size());
		for (unsigned j = 0; j < points_n; j++) {
			const Vector3& v = (*other_points)[j];
			mean_points[j].x += _param[obj_id] * (v.x - mean_points[j].x);
			mean_points[j].y += _param[obj_id] * (v.y - mean_points[j].y);
			mean_points[j].z += _param[obj_id] * (v.z - mean_points[j].z);
		}
	}

	out.delete_objects();
	out.add_object(0);
	out[0].copy(info_to_copy);
	PointList* out_points = out.writable_points(0);

	for (unsigned int j = 0; j < points_n; j++)
	{
		(*out_points)[j] = mean_points[j];
	}

}

void CombineMultPCA::knobs(Knob_Callback f)
{
	MultiFloat_knob(f, _param, N,"combination param", "combination param");
	SetRange(f, 0, 1);
}

static Op* build(Node* node)
{
	return new CombineMultPCA(node);
}

const Op::Description CombineMultPCA::description(CLASS, build);