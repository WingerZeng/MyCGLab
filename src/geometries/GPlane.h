#pragma once
#include "mcl.h"
#include "types.h"
#include "GTriangleMesh.h"
namespace mcl {
	class GPlane : public GTriangleMesh
	{
	public:
		GPlane(const Transform& toWorld);

	private:
		static std::vector<Point3f> planePts;
		static std::vector<Point2f> planeUvs;
		static std::vector<int> planeIndices;
	};
}

