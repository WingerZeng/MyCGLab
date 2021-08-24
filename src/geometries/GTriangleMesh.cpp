#include "GTriangleMesh.h"
#include "Sampler.h"
namespace mcl{
	
	GTriangleMesh::GTriangleMesh(const Transform& transform, const std::vector<int>& indices, const std::vector<Point3f>& points)
		:Geometry(transform),indices(indices),points(points),useUV(false),useNormal(false)
	{
		createBoundAndTree();
	}

	GTriangleMesh::GTriangleMesh(const Transform& transform, const std::vector<int>& indices, const std::vector<Point3f>& points, const std::vector<Point2f>& uvs, bool isUvPerElement)
		: Geometry(transform), indices(indices), points(points),uv(uvs), useUV(true), isUvPerElement(isUvPerElement), useNormal(false)
	{
		createBoundAndTree();
	}

	GTriangleMesh::GTriangleMesh(const Transform& transform, const std::vector<int>& indices, const std::vector<Point3f>& points, const std::vector<Point2f>& uvs, const std::vector<Normal3f>& normals, bool isUvPerElement /*= true*/)
		: Geometry(transform), indices(indices), points(points), uv(uvs), useUV(true), isUvPerElement(isUvPerElement), useNormal(true), normals(normals)
	{
		createBoundAndTree();
	}

	bool GTriangleMesh::intersect(const Ray& ray, HitRecord* rec)
	{
		if (octree) {
			Triangle htri;
			Ray localray = toLocal(ray);
			if (!octree->intersect(localray, htri)) return false;
			int hitindex = htri.index;
			Point2f triuv[3];
			Point3f tri[3]{ points[indices[hitindex * 3]],points[indices[hitindex * 3 + 1]] ,points[indices[hitindex * 3 + 2]] };
			
			getUv(hitindex, triuv);

			bool find = false;
			if (useNormal) {
				Normal3f normal[3];
				getPtNormal(hitindex, normal);
				if (IntersectTriangle(localray, rec, tri, triuv, normal)) {
					find = true;
				}
			}
			else {
				if (IntersectTriangle(localray, rec, tri, triuv)) {
					find = true;
				}
			}
			CHECK(find);
			rec->transform(toWorld);
			return true;
		}

		if (!ray.intersect(worldBound())) return false;
		Ray localray = toLocal(ray);
		bool find = false;

		for (int i = 0; i < indices.size(); i += 3) {
			Point3f tri[3]{ points[indices[i]],points[indices[i + 1]] ,points[indices[i + 2]] };

			Point2f triuv[3];
			getUv(i / 3, triuv);

			if (useNormal) {
				Normal3f normal[3];
				getPtNormal(i/3, normal);
				if (IntersectTriangle(localray, rec, tri, triuv,normal)) {
					localray.t1 = rec->t;
					find = true;
				}
			}
			else {
				if (IntersectTriangle(localray, rec, tri, triuv)) {
					localray.t1 = rec->t;
					find = true;
				}
			}
		}

		if (find) {
			rec->transform(toWorld);
		}
		return find;
	}

	bool GTriangleMesh::intersectShadow(const Ray& ray)
	{
		if (octree) {
			Triangle thit;
			return octree->intersect(toLocal(ray), thit);
		}
		if (!ray.intersect(worldBound())) return false;
		Ray localray = toLocal(ray);
		for (int i = 0; i < indices.size(); i += 3) {
			Point3f tri[3]{ points[indices[i]],points[indices[i + 1]] ,points[indices[i + 2]] };
			if (FastIntersectTriangle(ray, tri)) return true;
		}
		return false;
	}

	void GTriangleMesh::needSample()
	{
		Float totalArea = 0;
		areaPercentage.reserve(indices.size()/3);

		for (int i = 0; i < indices.size(); i += 3) {
			Point3f worldTri[3];
			for (int j = 0; j < 3; j++) {
				worldTri[j] = toWorld(points[indices[i + j]]);
			}
			areaPercentage.emplace_back(calTriArea(worldTri[0], worldTri[1], worldTri[2]));
			totalArea += areaPercentage[i/3];
		}

		invTotalArea = 1 / totalArea;

		Float sum = 0;
		for (auto& p : areaPercentage) {
			Float temp = p;
			p = sum * invTotalArea;
			sum += temp;
		}
	}

	mcl::SurfaceRecord GTriangleMesh::Sample(Sampler& sampler, Float& possibility)
	{
		//基于面积概率选择三角片
		Float p = sampler.get1DSample();
		int selectedTri = std::upper_bound(areaPercentage.begin(), areaPercentage.end(), p) - areaPercentage.begin() - 1;
		assert(selectedTri >= 0 && selectedTri < areaPercentage.size());
		
		//计算坐标与uv
		Point3f worldTri[3];
		for (int j = 0; j < 3; j++) {
			worldTri[j] = toWorld(points[indices[3 * selectedTri + j]]);
		}

		Point2f triuv[3];
		getUv(selectedTri, triuv);

		//在单个三角形上进行采样
		Point2f st = sampler.get2DSample();
		if (st[0] + st[1] > 1) {
			st[0] = 1 - st[0];
			st[1] = 1 - st[1];
		}

		SurfaceRecord rec;
		rec.pos = st[0] * (worldTri[1] - worldTri[0]) + st[1] * (worldTri[2] - worldTri[0]) + worldTri[0];
		rec.uv = triuv[0] + st[0] * (triuv[1] - triuv[0]) + st[1] * (triuv[2] - triuv[0]);
		calUvVec(worldTri[0], worldTri[1], worldTri[2], rec.n ,triuv[0], triuv[1], triuv[2], rec.uvec, rec.vvec);

		Normal3f normal[3];
		getPtNormal(selectedTri, normal);
		rec.n = (normal[1] - normal[0]) * st[0] + (normal[2] - normal[0]) * st[1] + normal[0];
		possibility = invTotalArea;
		return rec;
	}

	void GTriangleMesh::getUv(int tri, Point2f* triuv)
	{
		if (!useUV) {
			triuv[0] = Point2f(0, 0);
			triuv[1] = Point2f(1, 0);
			triuv[2] = Point2f(1, 1);
		}
		else {
			if (!isUvPerElement) {
				for (int j = 0; j < 3; j++)
				{
					triuv[j] = uv[indices[tri * 3 + j]];
				}
			}
			else {
				for (int j = 0; j < 3; j++)
				{
					triuv[j] = uv[tri * 3 + j];
				}
			}
		}
	}

	void GTriangleMesh::getPtNormal(int tri, Normal3f* trin)
	{
		if (useNormal) {
			trin[0] = normals[tri * 3 + 0];
			trin[1] = normals[tri * 3 + 1];
			trin[2] = normals[tri * 3 + 2];
		}
		else {
			Point3f pt1 = points[indices[tri * 3 + 0]];
			Point3f pt2 = points[indices[tri * 3 + 1]];
			Point3f pt3 = points[indices[tri * 3 + 2]];
			trin[0] = trin[1] = trin[2] = Normal3f((pt2 - pt1).cross(pt3 - pt1));
		}
	}

	void GTriangleMesh::createBoundAndTree()
	{
		for (const auto& pt : points) {
			wbd.unionPt(toWorld(pt));
		}

		int ntri = indices.size() / 3;
		if (ntri >= 16) {
			//创建八叉树
			octree = std::make_unique<Octree<Triangle, TriangleBoundFunc, TriangleIntersectFunc>>(6, 4);
			for (int i = 0; i < ntri; i++) {
				Triangle tri;
				tri.index = i;
				tri.wbd.unionPt(points[indices[3 * i]]);
				tri.wbd.unionPt(points[indices[3 * i + 1]]);
				tri.wbd.unionPt(points[indices[3 * i + 2]]);
				tri.mesh = this;
				octree->addElement(std::move(tri));
			}
			octree->createTree();
		}
	}

	bool IntersectTriangle(const Ray& ray, HitRecord* rec, Point3f tri[3], Point3f& cross)
	{
		Point3f rayTri[3]; //变换过后的三角片坐标
		//变换到光线起点为原点，光线方向为z方向的坐标系下
		//平移变换
		rayTri[0] = Point3f(tri[0] - ray.o);
		rayTri[1] = Point3f(tri[1] - ray.o);
		rayTri[2] = Point3f(tri[2] - ray.o);

		//坐标交换，将光线坐标绝对值最大维度交换到z方向
		int maxdim = ray.d.absMaxDim();
		if (maxdim != 2) {
			rayTri[0].swapDim(maxdim, 2);
			rayTri[1].swapDim(maxdim, 2);
			rayTri[2].swapDim(maxdim, 2);
		}
		Vector3f temprayd = ray.d;
		temprayd.swapDim(maxdim, 2);

		//剪切变换，将光线方向变换到z轴正方向、
		Float rayz = temprayd.z();
		Float invrayz = 1 / rayz;
		Float xshear = -temprayd.x() * invrayz;
		Float yshear = -temprayd.y() * invrayz;
		for (int i = 0; i < 3; i++) {
			rayTri[i].x() += xshear * rayTri[i].z();
			rayTri[i].y() += yshear * rayTri[i].z();
		}

#ifdef DEBUG
		std::cout << "Ray after rotation: " << ray.d << std::endl;
#endif

		Point2f ptri[3]; //triangle in plane
		for (int i = 0; i < 3; i++) {
			ptri[i] = Point2f(rayTri[i].x(), rayTri[i].y());
		}

		//计算每条边与原点的关系
		for (int i = 0; i < 3; i++) {
			cross[i] = ptri[(i + 1) % 3].x() * ptri[(i + 2) % 3].y() - ptri[(i + 1) % 3].y() * ptri[(i + 2) % 3].x();
		}

		//如果三条边到原点的叉乘异号，则三角形不包含原点
		if (!((cross[0] <= 0 && cross[1] <= 0 && cross[2] <= 0) || (cross[0] >= 0 && cross[1] >= 0 && cross[2] >= 0))) {
			return false;
		}
		//如果三个叉乘为0，说明光线与一条三角边重合，判定为不相交
		if (cross[0] == 0 && cross[1] == 0 && cross[2] == 0) {
			return false;
		}

		Float invSumCross = 1/(cross[0] + cross[1] + cross[2]);
		for (int i = 0; i < 3; i++) {
			cross[i] *= invSumCross;
		}

		//求交点t值
		Float t = rayTri[0].z() * cross[0] + rayTri[1].z() * cross[1] + rayTri[2].z() * cross[2];
		t *= invrayz;
		if ((t <ray.t0 ) || (t > ray.t1)) {
			return false;
		}
		//如果三角形退化为直线，则返回false
		auto normal = (tri[1] - tri[0]).cross(tri[2] - tri[1]);
		if (normal.length() == 0) return false;
		rec->t = t;
		rec->n = Normal3f(normal);
		rec->pos = ray.pos(rec->t);

		return true;
	}

	bool IntersectTriangle(const Ray& ray, HitRecord* rec, Point3f tri[3], Point2f uv[3])
	{
		Point3f cross;
		if (!IntersectTriangle(ray, rec, tri, cross))
			return false;

		//计算uv方向
		rec->uv = cross[0] * uv[0] + cross[1] * uv[1] + cross[2] * uv[2];
		calUvVec(tri[0], tri[1], tri[2], rec->n, uv[0] , uv[1], uv[2], rec->uvec, rec->vvec);
		
		return true;
	}

	bool IntersectTriangle(const Ray& ray, HitRecord* rec, Point3f tri[3], Point2f uv[3], Normal3f normal[3])
	{
		Point3f cross;
		if (!IntersectTriangle(ray, rec, tri, cross))
			return false;

		//计算uv方向
		rec->uv = cross[0] * uv[0] + cross[1] * uv[1] + cross[2] * uv[2];
		calUvVec(tri[0], tri[1], tri[2],rec->n, uv[0], uv[1], uv[2], rec->uvec, rec->vvec);
		rec->n = cross[0] * normal[0] + cross[1] * normal[1] + cross[2] * normal[2];

		return true;
	}

	bool FastIntersectTriangle(const Ray& ray, Point3f tri[3])
	{
		Float thit;
		return FastIntersectTriangle(ray, tri, thit);
	}

	bool FastIntersectTriangle(const Ray& ray, Point3f tri[3], Float& thit)
	{
		Float cross[3];
		Point3f rayTri[3]; //变换过后的三角片坐标
		//变换到光线起点为原点，光线方向为z方向的坐标系下
		//平移变换
		rayTri[0] = Point3f(tri[0] - ray.o);
		rayTri[1] = Point3f(tri[1] - ray.o);
		rayTri[2] = Point3f(tri[2] - ray.o);

		//坐标交换，将光线坐标绝对值最大维度交换到z方向
		int maxdim = ray.d.absMaxDim();
		if (maxdim != 2) {
			rayTri[0].swapDim(maxdim, 2);
			rayTri[1].swapDim(maxdim, 2);
			rayTri[2].swapDim(maxdim, 2);
		}
		Vector3f temprayd = ray.d;
		temprayd.swapDim(maxdim, 2);

		//剪切变换，将光线方向变换到z轴正方向、
		Float rayz = temprayd.z();
		Float invrayz = 1 / rayz;
		Float xshear = -temprayd.x() * invrayz;
		Float yshear = -temprayd.y() * invrayz;
		for (int i = 0; i < 3; i++) {
			rayTri[i].x() += xshear * rayTri[i].z();
			rayTri[i].y() += yshear * rayTri[i].z();
		}

		Point2f ptri[3]; //triangle in plane
		for (int i = 0; i < 3; i++) {
			ptri[i] = Point2f(rayTri[i].x(), rayTri[i].y());
		}

		//计算每条边与原点的关系
		for (int i = 0; i < 3; i++) {
			cross[i] = ptri[(i + 1) % 3].x() * ptri[(i + 2) % 3].y() - ptri[(i + 1) % 3].y() * ptri[(i + 2) % 3].x();
		}

		//如果三条边到原点的叉乘异号，则三角形不包含原点
		if (!((cross[0] <= 0 && cross[1] <= 0 && cross[2] <= 0) || (cross[0] >= 0 && cross[1] >= 0 && cross[2] >= 0))) {
			return false;
		}
		//如果三个叉乘为0，说明光线与一条三角边重合，判定为不相交
		if (cross[0] == 0 && cross[1] == 0 && cross[2] == 0) {
			return false;
		}

		Float invSumCross = 1 / (cross[0] + cross[1] + cross[2]);
		for (int i = 0; i < 3; i++) {
			cross[i] *= invSumCross;
		}

		//求交点t值
		Float t = rayTri[0].z() * cross[0] + rayTri[1].z() * cross[1] + rayTri[2].z() * cross[2];
		t *= invrayz;
		if ((t < ray.t0) || (t > ray.t1)) {
			return false;
		}
		thit = t;
		//如果三角形退化为直线，则返回false
		auto normal = (tri[1] - tri[0]).cross(tri[2] - tri[1]);
		if (normal.length() == 0) return false;
		return true;
	}

	bool GTriangleMesh::TriangleIntersectFunc::operator()(const GTriangleMesh::Triangle& tri, const Ray& ray, Float& thit)
	{
		Point3f trip[3];
		trip[0] = tri.mesh->points[tri.mesh->indices[tri.index * 3]];
		trip[1] = tri.mesh->points[tri.mesh->indices[tri.index * 3 + 1]];
		trip[2] = tri.mesh->points[tri.mesh->indices[tri.index * 3 + 2]];
		return FastIntersectTriangle(ray, trip, thit);
	}

}