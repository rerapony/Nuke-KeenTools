#include "PCAGeo.hpp"
#include <vector>

using namespace DD::Image;

namespace {
    const char* const CLASS = "PCAGeo";
    const char* const HELP = "Combine geometries of two objects";
}


void PCAGeo::_validate(bool for_real)
{
    int current_inputs_n = 0;
    for (int i = 0; i < max_inputs_N; i++)
    {
        if (Op::input(i) != nullptr)
        {
            Op::input(i)->validate(for_real);
            current_inputs_n++;
        }
    }
    inputs_N = current_inputs_n;
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

PCAGeo::PCAGeo(Node* node):
    GeoOp(node),
    max_inputs_N(10),
    inputs_N(0),
    points_N(0),
    objs_N(0),
    mid_obj_id(0),
    min_pca_N(1),
    pretty_show(false),
    var_threshold(0.2),
    d_x(2) {}

int PCAGeo::minimum_inputs() const
{
    return 2;
}

int PCAGeo::maximum_inputs() const 
{
    return max_inputs_N;
}

void PCAGeo::get_geometry_hash()
{
    GeoOp::get_geometry_hash();
    geo_hash[Group_Points].append(min_pca_N);
    geo_hash[Group_Points].append(pretty_show);
    geo_hash[Group_Points].append(var_threshold);
    geo_hash[Group_Points].append(d_x);
}

void PCAGeo::geometry_engine(Scene& scene, GeometryList& out)
{
    GeometryList in;
    {
        Scene in_scene;
        input(0)->get_geometry(in_scene, in);
    }
    objs_N = in.objects();
    assert(objs_N > 0);
    points_N = in[0].points();

    auto result_vec = prepare_data();

    Pca pca;
    const int init_result = pca.Calculate(result_vec, inputs_N, objs_N * points_N * 3);
    assert(init_result == 0);

    process_extreme_points(pca, out, &in[0]);
}

std::vector<float> PCAGeo::prepare_data() const
{
    std::vector<float> result_vec;
    for (int geo_id = 0; geo_id < inputs_N; geo_id++)
    {
        GeometryList in;
        {
            Scene in_scene;
            input(geo_id)->get_geometry(in_scene, in);
        }

        for (unsigned int obj_id = 0; obj_id < objs_N; obj_id++) {

            GeoInfo& info = in[obj_id];
            const PointList* points = info.point_list();

            for (unsigned int j = 0; j < points_N; j++) {
                const Vector3& v = (*points)[j];
                result_vec.push_back(v.x);
                result_vec.push_back(v.y);
                result_vec.push_back(v.z);
            }
        }
    }
    assert(result_vec.size() == inputs_N * objs_N * points_N * 3);
    return result_vec;
}

void PCAGeo::process_extreme_points(Pca& pca, GeometryList& out, GeoInfo*  info_to_copy)
{
    int pca_n = pca.pca_size().first;
    int thresh_i = min_pca_N >= 0 ? min(min_pca_N, pca_n) : 0;
    std::vector<float> cum_props = pca.var_proportions();

    for ( ; thresh_i < pca_n; thresh_i++)
    {
        if (cum_props[thresh_i] < var_threshold)
        {
            break;
        }
    }
    pca_n = thresh_i;
    mid_obj_id = pca_n / 2;

    out.delete_objects();
    std::vector<float> mean = pca.mean();
    write_neutral_model(mean, out, info_to_copy);

    auto extreme_points = pca.calculate_extreme_points(pca_n);
    write_pca_models(extreme_points, pca_n, out, info_to_copy);
}

void PCAGeo::write_neutral_model(std::vector<float>& mean, GeometryList& out, GeoInfo*  info) const
{
    out.add_object(0);
    out[0].copy(info);
    PointList* points = out.writable_points(0);

    for (unsigned int j = 0; j < points_N; j++)
    {
        Vector3& v = (*points)[j];
        v.x = mean[3 * j];
        v.y = mean[3 * j + 1];
        v.z = mean[3 * j + 2];
    }

    if (pretty_show) {
        out[0].matrix.translate(mid_obj_id*d_x, 0, 0);
    }
}

void PCAGeo::write_pca_models(
    std::vector<std::vector<float>>& pca_points, 
    int pca_n, 
    GeometryList& out, 
    GeoInfo* info) const
{
    for (int pca_id = 1; pca_id < pca_n+1; pca_id++)
    {
        out.add_object(pca_id);
        out[pca_id].copy(info);
        PointList* points = out.writable_points(pca_id);
        std::vector<float> current_obj = pca_points[pca_id-1];

        for (unsigned int j = 0; j < points_N; j++)
        {
            Vector3& v = (*points)[j];
            v.x = current_obj[3 * j];
            v.y = current_obj[3 * j + 1];
            v.z = current_obj[3 * j + 2];
        }

        if (pretty_show) {
            out[pca_id].matrix.translate((mid_obj_id - pca_id)*d_x, 0, 0);
        }
    }
}

void PCAGeo::knobs(Knob_Callback f)
{
    Bool_knob(f, &pretty_show, "show PCA components in a row", "Pretty Show");
    Float_knob(f, &d_x, "delta x", "Delta X");
    SetRange(f, 0, 10);
    Int_knob(f, &min_pca_N, "minimum number of PCA to be shown", "N_PCA");
    SetRange(f, 0, max_inputs_N);
    Float_knob(f, &var_threshold, "variance threshold", "Variance Threshold");
    SetRange(f, 0, 1);
}

namespace { 
    Op* build(Node* node)
    {
        return new PCAGeo(node);
    }
}

const Op::Description PCAGeo::description(CLASS, build);