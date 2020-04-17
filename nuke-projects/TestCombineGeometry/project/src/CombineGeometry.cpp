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
			error("Can't work with one geometry.");

		Scene other_scene;
		GeometryList other;
		input1()->get_geometry(other_scene, other);

		unsigned int objs = out.objects();
		unsigned int other_objs = other.objects();
		assert(objs == other_objs);

		std::cout << objs;

		for (unsigned int i = 0; i < objs; ++i)
		{
			PointList* points = out.writable_points(i);
			GeoInfo& other_info = other[i];
			const PointList* other_points = other_info.point_list();

			const unsigned n = points->size();
			const unsigned other_n = other_points->size();

			std::cout << n;

			assert(n == other_n);

			for (unsigned j = 0; j < n; j++) {
				Vector3& v = (*points)[j];
				const Vector3& other_v = (*other_points)[j];
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