#pragma once
#include "mcl.h"
#include "types.h"
#include "Geometry.h"
#include "OcTree.h"
namespace mcl {
	class GTriangleMesh: public Geometry
	{
	public:
		GTriangleMesh(const Transform& toWorld, const std::vector<int>& indices, const std::vector<Point3f>& points);
		GTriangleMesh(const Transform& toWorld, const std::vector<int>& indices, const std::vector<Point3f>& points, const std::vector<Point2f>& uvs, bool isUvPerElement = true);
		GTriangleMesh(const Transform& toWorld, const std::vector<int>& indices, const std::vector<Point3f>& points, const std::vector<Point2f>& uvs, const std::vector<Normal3f>& normals,bool isUvPerElement = true);
		bool intersect(const Ray& ray, HitRecord* rec) override;
		bool intersectShadow(const Ray& ray) override;
		Bound3f worldBound() override { return wbd; }
		void needSample() override;
		SurfaceRecord Sample(Sampler& sampler, Float& possibility) override;
		SamplerRequestInfo getSamplerRequest() override {
			return SamplerRequestInfo(1, 1);
		}

	private:
		void getUv(int tri, Point2f* triuv);
		void getPtNormal(int tri, Normal3f* trin);

		void createBoundAndTree();
		Bound3f wbd;

		std::vector<int> indices;	// dim = 3 * ntri
		std::vector<Normal3f> normals; // dim = 3 * ntri
		std::vector<Point3f> points;
		std::vector<Point2f> uv;

		bool useNormal;
		bool useUV;
		bool isUvPerElement; // 如果该项为false，则uv绑定在顶点上，否则绑定在三角片上

		std::vector<Float> areaPercentage; //areaPercentage[i]: 前i个三角形的面积占总面积百分比
		Float invTotalArea;

		struct Triangle
		{
			Bound3f wbd;
			int index;
			GTriangleMesh* mesh;
		};

		class TriangleBoundFunc
		{
		public:
			Bound3f operator()(const Triangle& tri) { return tri.wbd; }
		};

		class TriangleIntersectFunc
		{
		public:
			bool operator()(const Triangle& tri, const Ray& ray, Float& thit);
		};
		friend TriangleIntersectFunc;

		std::unique_ptr < Octree<Triangle, TriangleBoundFunc,TriangleIntersectFunc>> octree;
	};

	/*
	@brief 三角形求交算法，cross返回到三个点的权重
	*/
	bool IntersectTriangle(const Ray& ray, HitRecord* rec, Point3f tri[3], Point3f& cross);
	bool FastIntersectTriangle(const Ray& ray, Point3f tri[3]);
	bool FastIntersectTriangle(const Ray& ray, Point3f tri[3], Float& thit);
	bool IntersectTriangle(const Ray& ray, HitRecord* rec, Point3f tri[3], Point2f uv[3]);
	bool IntersectTriangle(const Ray& ray, HitRecord* rec, Point3f tri[3], Point2f uv[3], Normal3f normal[3]);
	bool IntersectTriangle(const Ray& ray, HitRecord* rec, Point3f tri[3]);

	inline Float calTriArea(const Point3f& p1, const Point3f& p2, const Point3f& p3) {
		return std::abs((p2 - p1).cross(p3 - p1).length() / 2);
	}

	inline void calUvVec(const Point3f& p1, const Point3f& p2, const Point3f& p3, const Normal3f& n, const Point2f& uv1, const Point2f& uv2, const Point2f& uv3, Vector3f& uvec, Vector3f& vvec) {
		Vector3f e1 = p2 - p1;
		Vector3f e2 = p3 - p1;
		Vector2f duv1 = uv2 - uv1;
		Vector2f duv2 = uv3 - uv1;

		//退化情况时，指定边为uv方向
		Float c = std::abs(duv1.x()*duv2.y() - duv1.y()*duv2.x());
		if (std::abs(c) < FloatZero) {
			uvec = Vector3f(0, 1, 0).cross(Vector3f(n));
			if (uvec.length() < FloatZero) {
				uvec = e1;
				vvec = e2;
			}
			else {
				vvec = Vector3f(n).cross(uvec);
				uvec.normalize();
				vvec.normalize();
			}
		}
		else {
			Float invc = 1 / c;
			uvec = (e1 * duv2.y() - e2 * duv1.y()) * invc;
			vvec = (e1 * -duv2.x() + e2 * duv1.x()) * invc;
		}
	}
}

