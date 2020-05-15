#include "PCAGeo.hpp"

#include <vector>
#include <cmath>

void PCAGeo::_validate(bool for_real)
{
	for (int i = 0; i < N; i++)
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

PCAGeo::PCAGeo(Node* node) : GeoOp(node)
{
	N_pca = 1; // only first N_pca ordered PCA components will be shown
	pretty_show = false; // should position_delta be applied to all the models for a good exposure
	var_threshold = 0.2f; // only PCA with cumulative variance proportion >= threshold will be loaded
	d_x = 2;
}

int PCAGeo::minimum_inputs() const
{
	return 2;
}

int PCAGeo::maximum_inputs() const 
{
	return N;
}

void PCAGeo::get_geometry_hash()
{
	GeoOp::get_geometry_hash();
	geo_hash[Group_Points].append(N_pca);
	geo_hash[Group_Points].append(pretty_show);
	geo_hash[Group_Points].append(var_threshold);
	geo_hash[Group_Points].append(d_x);
}

void PCAGeo::geometry_engine(Scene& scene, GeometryList& out) {

	std::vector<float> result_vec;
	unsigned int total_n = 0;
	unsigned int total_m = 0;
	unsigned points_n = 0;

	Scene other_scene;
	GeometryList other;
	input(0)->get_geometry(other_scene, other);

	prepare_data(result_vec, total_n, total_m);
	
	Pca *pca = new Pca();
	const int init_result = pca->Calculate(result_vec, total_n, total_m);

	assert(init_result == 0);

	calculate_extreme_points(pca, out, &other[0]);
	
	delete pca;
}

void PCAGeo::prepare_data(std::vector<float>& data_vec, unsigned int& n, unsigned int& m) const
{
	unsigned int points_n = 0;
	
	for (int geo_id = 0; geo_id < N + 1; geo_id++)
	{
		if (Op::input(geo_id) == nullptr)
			break;

		std::vector<float> model_vec;

		Scene other_scene;
		GeometryList other;
		input(geo_id)->get_geometry(other_scene, other);

		const unsigned int objs = other.objects();

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

			m = model_vec.size();
			data_vec.insert(data_vec.end(), model_vec.begin(), model_vec.end());
			n++;
		}
	}
}

void PCAGeo::calculate_extreme_points(Pca* pca, GeometryList& out, GeoInfo*  info_to_copy) const
{
	std::vector<float> lengths = pca->pca_variance();
	std::vector<std::vector<float>> components = pca->pca_components();
	std::vector<float> cum_props = pca->var_proportions();
	std::vector<float> mean = pca->mean();
	int n_pca = pca->pca_size().first;

	int thresh_i = N_pca >= 0 ? min(N_pca, n_pca) : 0;
	for ( ; thresh_i < n_pca; thresh_i++)
	{
		if (cum_props[thresh_i] < var_threshold)
		{
			break;
		}
	}

	n_pca = thresh_i;
	std::vector<std::vector<float>> result_points(n_pca);
	// initialize result models
	for (int i = 0; i < n_pca; i++)
	{
		std::vector<float> extreme_point = sqrt(lengths[i])*components[i];
		const std::vector<float> res = pca->mean() + extreme_point;
		result_points[i] = res;
	}

	out.delete_objects();
	const int mid_object = n_pca / 2;

	for (int pca_id = 0; pca_id < n_pca; pca_id++)
	{
		out.add_object(pca_id);
		out[pca_id].copy(info_to_copy);
		PointList* points = out.writable_points(pca_id);

		const unsigned int points_n = points->size();
		std::vector<float> current_obj = result_points[pca_id];

		for (unsigned j = 0; j < points_n; j++)
		{
			Vector3& v = (*points)[j];
			v.x = current_obj[3 * j];
			v.y = current_obj[3 * j + 1];
			v.z = current_obj[3 * j + 2];

			// row aligning
			if (pretty_show)
			{	// small changes in face?
				if (pca_id <= mid_object)
				{
					v.x -= (mid_object - pca_id)*d_x;
				}
				else
				{
					v.x += (pca_id - mid_object)*d_x;
				}
			}
		}
	}
}

void PCAGeo::knobs(Knob_Callback f)
{
	Bool_knob(f, &pretty_show, "show PCA components in a row", "Pretty Show");
	Float_knob(f, &d_x, "delta x", "Delta X");
	SetRange(f, 0, 10);
	Int_knob(f, &N_pca, "minimum number of PCA to be shown", "N_PCA");
	SetRange(f, 0, N);
	Float_knob(f, &var_threshold, "variance threshold", "Variance Threshold");
	SetRange(f, 0, 1);
}

static Op* build(Node* node)
{
	return new PCAGeo(node);
}

const Op::Description PCAGeo::description(CLASS, build);