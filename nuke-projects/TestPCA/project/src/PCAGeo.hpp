#pragma once

#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/Knobs.h"
#include "DDImage/ViewFrustum.h"
#include "EigenPCA-master/pca.cpp"
#include <cassert>

class PCAGeo : public DD::Image::GeoOp
{
public:
    static const Description description;
    
    const char* Class() const override;
    
    const char* node_help() const override;

    PCAGeo(Node* node);

    int minimum_inputs() const override;
    
    int maximum_inputs() const override;

    void get_geometry_hash() override;

    void geometry_engine(DD::Image::Scene& scene, DD::Image::GeometryList& out) override;

    void knobs(DD::Image::Knob_Callback f) override;

protected:
    void _validate(bool for_real) override;
    
private:
    int max_inputs_N;
    int inputs_N;
    unsigned int points_N;
    unsigned int objs_N;
    int mid_obj_id;
    int min_pca_N;
    bool pretty_show;
    float var_threshold;
    float d_x;

    std::vector<float> PCAGeo::prepare_data() const;

    void process_extreme_points(Pca& pca, DD::Image::GeometryList& out, DD::Image::GeoInfo* info_to_copy);

    void write_neutral_model(std::vector<float>& mean, DD::Image::GeometryList& out, DD::Image::GeoInfo* info) const;

    void write_pca_models(std::vector<std::vector<float>>& pca_points,
        int pca_n, 
        DD::Image::GeometryList& out, 
        DD::Image::GeoInfo* info) const;
};
