#include "GSphere.h"
#include "algorithms.h"
#include "Record.h"
namespace mcl{
	
	GSphere::GSphere(Point3f center, Float radius, const Transform& tran)
		:Geometry(tran * Transform::translate(Vector3f(center))*Transform::scale(Vector3f(radius)))
	{
		invArea = 0.25 * INVPI / radius / radius;
	}

	bool GSphere::intersect(const Ray& worldray, HitRecord* rec)
	{
		Ray ray = toLocal(worldray);

		Float dDotO = ray.d.dot(Vector3f(ray.o));
		Float dieta = pow(dDotO, 2) - ray.d .lengthSquared() * (ray.o.lengthSquared() - 1);
		Float deno = ray.d.length();
		Float invDeno = 1 / deno;
		Float pre = -dDotO;

		if (abs(dieta) < FloatZero) {
			rec->t = pre * invDeno;
		}
		else if (dieta > 0) {
			Float sqrtDieta = sqrt(dieta);
			Float pt = (pre - sqrtDieta) * invDeno;
			Float nt = (pre + sqrtDieta) * invDeno;
			bool f1 = false;
			bool f2 = false;
			if (pt > ray.t1 || pt < ray.t0) f1 = true;
			if (nt > ray.t1 || nt < ray.t0) f2 = true;

			if (f1 && f2) {
				return false;
			}
			if (!f1) {
				rec->t = pt;
			}
			else {
				rec->t = nt;
			}
		}
		else{
			return false;
		}

		calRecord(ray.pos(rec->t), *rec);

		rec->transform(toWorld);

		return true;
	}

	mcl::Bound3f GSphere::worldBound()
	{
		//#TODO2
		return Bound3f();
	}

	mcl::SurfaceRecord GSphere::Sample(Sampler& sampler, Float& possibility)
	{
		Point3f p = uniformSampleSphere(sampler.get2DSample());
		SurfaceRecord srec;
		calRecord(p, srec);
		possibility = invArea;
		srec.transform(toWorld);
		return srec;
	}

	void GSphere::calRecord(const Point3f& pos, SurfaceRecord& rec)
	{
		DCHECK(FloatEq(pos.length(), 1));
		rec.pos = pos;
		//如果刚好为（0，0，1）偏移一个小角度
		if (FloatEq(abs(rec.pos.z()), 1)) {
			rec.pos.x() += RayTraceZero;
			rec.pos.y() += RayTraceZero;
		}
		rec.pos.normalize();
		rec.n = Normal3f(rec.pos);
		rec.uv = calUvInSphere(Vector3f(rec.pos));
		rec.uvec = Vector3f(-rec.pos.y() * PI_2, rec.pos.x() * PI_2, 0);
		rec.vvec = Normalize(Vector3f(rec.n).cross(rec.uvec)) * PI;
	}


}