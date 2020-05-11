#include "PCAGeo.hpp"

#include <vector>
#include <cmath>
#include "EigenPCA-master/pca.cpp"

void PCAGeo::_validate(bool for_real)
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

const char* PCAGeo::Class() const
{
	return CLASS;
}
const char* PCAGeo::node_help() const
{
	return HELP;
}

PCAGeo::PCAGeo(Node* node) : GeoOp(node) {
	for (int i = 0; i < N; ++i)
	{
		_param[i] = 0.5f;
	}
}

int PCAGeo::minimum_inputs() const
{
	return 1;
}

int PCAGeo::maximum_inputs() const 
{
	return N+1;
}

void PCAGeo::get_geometry_hash()
{
	GeoOp::get_geometry_hash();

	geo_hash[Group_Points].append(_param, N);
}

void PCAGeo::geometry_engine(Scene& scene, GeometryList& out) {

	std::vector<float> result_vec;
	unsigned int total_n = 0;
	unsigned int total_m = 0;
	unsigned points_n = 0;

	for (int geo_id = 0; geo_id < N+1; geo_id++)
	{
		if (Op::input(geo_id) == nullptr)
			break;

		std::vector<float> model_vec;
		
		Scene other_scene;
		GeometryList other;
		input(geo_id)->get_geometry(other_scene, other);

		unsigned int objs = other.objects();

		for (int obj_id = 0; obj_id < objs; obj_id++) {
			GeoInfo& info = other[obj_id];
			const PointList* points = info.point_list();

			points_n = points->size();

			for (unsigned j = 0; j < points_n; j++) {
				const Vector3& v = (*points)[j];

				model_vec.push_back(v.x);
				model_vec.push_back(v.y);
				model_vec.push_back(v.z);
			}

			total_m = model_vec.size();
			result_vec.insert(result_vec.end(), model_vec.begin(), model_vec.end());
			total_n++;
		}
	}

	Pca *pca = new Pca();
	int init_result = pca->Calculate(result_vec, total_n, total_m); 

	assert(init_result == 0);

	auto n_pca = pca->pca_size().first;
	auto lengths = pca->pca_variance();
	auto components = pca->pca_components();
	auto mean = pca->mean();


	
	delete pca;
}

void PCAGeo::knobs(Knob_Callback f)
{
	MultiFloat_knob(f, _param, N,"combination param", "combination param");
	SetRange(f, 0, 1);
}

static Op* build(Node* node)
{
	return new PCAGeo(node);
}

const Op::Description PCAGeo::description(CLASS, build);