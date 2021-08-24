#pragma once
#include "mcl.h"
#include "types.h"
#include "GTriangleMesh.h"

namespace mcl {
	class GCubeBox: public GTriangleMesh
	{
	public:
		GCubeBox(const Transform& toWorld);

	private:
		static std::vector<Point3f> cubePts;
		static std::vector<Point2f> cubeUvs;
		static std::vector<int> cubeIndices;
	};
}
