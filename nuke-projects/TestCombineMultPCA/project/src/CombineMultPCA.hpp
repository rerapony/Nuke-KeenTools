#pragma once

#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/Knobs.h"
#include "DDImage/ViewFrustum.h"
#include <cassert>


class CombineMultPCA : public DD::Image::GeoOp
{
public:
    static const Description description;

    const char* Class() const override;

    const char* node_help() const override;

    CombineMultPCA(Node* node);

    int minimum_inputs() const override;

    int maximum_inputs() const override;

    void get_geometry_hash() override;

    void geometry_engine(DD::Image::Scene& scene, DD::Image::GeometryList& out) override;
    
    void knobs(DD::Image::Knob_Callback f) override;

    ~CombineMultPCA();

protected:
    void _validate(bool for_real) override;

private:
    int pca_N;
    unsigned int points_n;
    int obj_n;
    float* params;

    void combine_pca(DD::Image::GeometryList& out, const DD::Image::GeometryList& in) const;
};