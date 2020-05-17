#include "CombineMultPCA.hpp"

#include <vector>

using namespace DD::Image;

namespace {
    const char* const CLASS = "CombineMultPCA";
    const char* const HELP = "Combine geometries of two objects";
}

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

CombineMultPCA::CombineMultPCA(Node* node):
    GeoOp(node),
    pca_N(10),
    points_n(0),
    obj_n(0),
    params(new float[pca_N])
{
    for (int i = 0; i < pca_N; i++)
    {
        params[i] = 0;
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
    geo_hash[Group_Points].append(params, pca_N);
}

void CombineMultPCA::geometry_engine(Scene& scene, GeometryList& out)
{
    GeometryList in;
    {
        Scene in_scene;
        input0()->get_geometry(scene, in);
    }
    obj_n = in.objects();
    assert(obj_n > 0 && obj_n <= pca_N+1);

    points_n = in[0].points();
    assert(points_n > 0);

    combine_pca(out, in);
}

void CombineMultPCA::combine_pca(GeometryList& out, const GeometryList& in) const
{
    const GeoInfo* info_to_copy = &in[0];
    
    out.delete_objects();
    out.add_object(0);
    out[0].copy(info_to_copy);
    PointList* out_points = out.writable_points(0);

    const GeoInfo& mean_info = in[0];
    const PointList* mean_points = mean_info.point_list();

    for (unsigned int j = 0; j < points_n; j++) {
        (*out_points)[j] = (*mean_points)[j];
    }

    for (int obj_id = 1; obj_id < obj_n; obj_id++) {
        const GeoInfo& other_info = in[obj_id];
        const PointList* other_points = other_info.point_list();
        assert(points_n == other_points->size());

        for (unsigned int j = 0; j < points_n; j++) {
            (*out_points)[j] += ((*other_points)[j] - (*mean_points)[j]) * params[obj_id-1];
        }
    }
}

void CombineMultPCA::knobs(Knob_Callback f)
{
    for (int i = 0; i < pca_N; i++)
    {
        Float_knob(f, &params[i], "combination param", "combination param");
        SetRange(f, -5,5);
    }
}

CombineMultPCA::~CombineMultPCA()
{
    delete[] params;
}

namespace {
    Op* build(Node* node)
    {
        return new CombineMultPCA(node);
    }
}

const Op::Description CombineMultPCA::description(CLASS, build);