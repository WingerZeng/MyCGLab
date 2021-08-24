#include "Geometry.h"

namespace mcl{
	
	Geometry::Geometry(const Transform& transform)
		:toLocal(transform.inverse()),toWorld(transform)
	{
	}

	bool Geometry::intersectShadow(const Ray& ray)
	{
		HitRecord rec;
		return intersect(ray, &rec);
	}

	mcl::SurfaceRecord Geometry::Sample(const HitRecord& rec, Sampler& sampler, Float& possibility)
	{
		SurfaceRecord srec = Sample(sampler, possibility);
		srec.transform(toWorld);
		//Ҫ����toworld scale������possibility
		assert(srec.n.normalized());

		Vector3f lvec = srec.pos - rec.pos;
		Float dist2 = lvec.lengthSquared();

		//#BUG �˴�dist��С�������
		possibility *= dist2 / Normalize(lvec).absDot(Vector3f(srec.n));

		return srec;
	}

}