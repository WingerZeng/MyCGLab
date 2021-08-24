#include "GPlane.h"

namespace mcl{
	
	GPlane::GPlane(const Transform& toWorld)
		:GTriangleMesh(toWorld, planeIndices,planePts,planeUvs,false)
	{

	}

	std::vector<mcl::Point3f> GPlane::planePts = std::vector<mcl::Point3f>{
		Point3f(-1,-1,0),
		Point3f(1,-1,0),
		Point3f(1,1,0),
		Point3f(-1,1,0)
	};

	std::vector<mcl::Point2f> GPlane::planeUvs = std::vector<mcl::Point2f>{
		Point2f(0,0),
		Point2f(1,0),
		Point2f(1,1),
		Point2f(0,1)
	};

	std::vector<int> GPlane::planeIndices = std::vector<int>{
		0, 1, 3,
		1, 2, 3
	};
}