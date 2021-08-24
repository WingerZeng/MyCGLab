#include "GCubeBox.h"

namespace mcl{
	
	GCubeBox::GCubeBox(const Transform& transform)
		:GTriangleMesh(transform,cubeIndices,cubePts, cubeUvs)
	{
	}

	std::vector<mcl::Point3f> GCubeBox::cubePts = std::vector<mcl::Point3f>{
		mcl::Point3f(-1,-1,-1),
		mcl::Point3f(1,-1,-1),
		mcl::Point3f(-1,1,-1),
		mcl::Point3f(1,1,-1),
		mcl::Point3f(-1,-1,1),
		mcl::Point3f(1,-1,1),
		mcl::Point3f(-1,1,1),
		mcl::Point3f(1,1,1),
	};

	std::vector<mcl::Point2f> GCubeBox::cubeUvs = std::vector<mcl::Point2f>{
		// bottom
		mcl::Point2f(0,0),mcl::Point2f(1,0),mcl::Point2f(0,1),
		mcl::Point2f(0,1),mcl::Point2f(1,0),mcl::Point2f(1,1),
		// top
		mcl::Point2f(0,0),mcl::Point2f(1,0),mcl::Point2f(0,1),
		mcl::Point2f(1,0),mcl::Point2f(1,1),mcl::Point2f(0,1),
		// front
		mcl::Point2f(0,0),mcl::Point2f(0,1),mcl::Point2f(1,0),
		mcl::Point2f(0,1),mcl::Point2f(1,1),mcl::Point2f(1,0),
		// back
		mcl::Point2f(1,0),mcl::Point2f(1,1),mcl::Point2f(0,0),
		mcl::Point2f(0,0),mcl::Point2f(1,1),mcl::Point2f(0,1),
		// left
		mcl::Point2f(1,0),mcl::Point2f(1,1),mcl::Point2f(0,0),
		mcl::Point2f(0,0),mcl::Point2f(1,1),mcl::Point2f(0,1),
		// right
		mcl::Point2f(0,0),mcl::Point2f(1,0),mcl::Point2f(0,1),
		mcl::Point2f(1,0),mcl::Point2f(1,1),mcl::Point2f(0,1)
	};

	std::vector<int> GCubeBox::cubeIndices = std::vector<int>{
		// bottom
		0,2,1,
		1,2,3,
		// top
		4,5,6,
		5,7,6,
		// front
		0,1,4,
		1,5,4,
		// back
		2,6,3,
		3,6,7,
		//left
		0,4,2,
		2,4,6,
		//right
		1,3,5,
		3,7,5};


}