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
	
	GeoInfo* info_to_copy = &other[0];
	const unsigned int points_n = info_to_copy->point_list()->size();
	std::vector<Vector3> mean_points(points_n, Vector3(0, 0,0));
	// average vectors
	make_means(mean_points, other);

	out.delete_objects();
	out.add_object(0);
	out[0].copy(info_to_copy);
	PointList* out_points = out.writable_points(0);
	
	for (unsigned int obj_id = 0; obj_id < objs; obj_id++) {
		GeoInfo& other_info = other[obj_id];
		const PointList* other_points = other_info.point_list();
		assert(points_n == other_points->size());
		for (unsigned int j = 0; j < points_n; j++) {
			const Vector3 v = (*other_points)[j];
			Vector3& out_v = (*out_points)[j];
			out_v = mean_points[j] + (v - mean_points[j])*_param[obj_id];
		}
	}
}

void CombineMultPCA::make_means(std::vector<Vector3>& mean_vec, GeometryList& geo_list)
{
	const unsigned int n = mean_vec.size();
	const unsigned int n_obj = geo_list.objects();

	for (unsigned int obj_id = 0; obj_id < n_obj; obj_id++)
	{
		GeoInfo& info = geo_list[obj_id];
		const PointList* points = info.point_list();
		assert(n == points->size());

		for (unsigned int j = 0; j < n; j++) {
			const Vector3 v = (*points)[j];
			mean_vec[j] += v;
		}
	}
	
	// average points
	for (unsigned int j = 0; j < n; j++) {
		mean_vec[j] /= (float)n_obj;
	}
}

void CombineMultPCA::knobs(Knob_Callback f)
{
	MultiFloat_knob(f, _param, N,"combination param", "combination param");
	SetRange(f, 0, 10);
}

static Op* build(Node* node)
{
	return new CombineMultPCA(node);
}

const Op::Description CombineMultPCA::description(CLASS, build);