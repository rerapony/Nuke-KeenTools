#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include <DDImage/SourceGeo.h>
#include "DDImage/ViewFrustum.h"

#include <cassert>

using namespace DD::Image;

static const char* const CLASS = "CombineGeometry";
static const char* const HELP = "Combine geometries of two objects";

class CombineGeometry : public GeoOp
{

protected:
	void _validate(bool for_real) override
	{
		Op* this_op = Op::input(0);
		Op* other_op = Op::input(1);

		this_op->validate(for_real);
		other_op->validate(for_real);
		
		GeoOp::_validate(for_real);
	}

public:
	static const Description description;
	const char* Class() const override
	{
		return CLASS;
	}
	const char* node_help() const override
	{
		return HELP;
	}

	CombineGeometry(Node* node) : GeoOp(node) {}

	int minimum_inputs() const override
	{
		return 2;
	}
	int maximum_inputs() const override
	{
		return 2;
	}

	void get_geometry_hash() override
	{
		GeoOp::get_geometry_hash();
	}

	void geometry_engine(Scene& scene, GeometryList& out) override
	{
		input0()->get_geometry(scene, out);

		if (Op::input(1) == nullptr)
			return;
		
		Scene other_scene;
		GeometryList other_out;
		input1()->get_geometry(other_scene, other_out);

		// вообще и там, и там должен быть один объект...
		unsigned int objs = out.objects();
		unsigned int other_objs = other_out.objects();
		unsigned int min_objs = MIN(objs, other_objs);
		
		for (unsigned int i = 0; i < min_objs; ++i)
		{
			PointList* points = out.writable_points(i);
			// вылетает здесь: unable to read memory
			PointList* other_points = other_out.writable_points(i);

			const unsigned n = points->size();
			const unsigned other_n = other_points->size();

			if (n != other_n)
				return;
			
			for (unsigned j = 0; j < n; j++) {
				Vector3& v = (*points)[j];
				Vector3& other_v = (*other_points)[j];
				v.x = (v.x + other_v.x) / 2;
				v.y = (v.y + other_v.y) / 2;
				v.z = (v.z + other_v.z) / 2;
			}
		}
	}
};

static Op* build(Node* node)
{
	return new CombineGeometry(node);
}
const Op::Description CombineGeometry::description(CLASS, build);
