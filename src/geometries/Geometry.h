#pragma once
#include "mcl.h"
#include "types.h"
#include "raytrace/Record.h"
#include "Sampler.h"
namespace mcl {
	class Geometry
	{
	public:
		Geometry(const Transform& toWorld);
		virtual bool intersect(const Ray& ray, HitRecord* rec) = 0;
		virtual bool intersectShadow(const Ray& ray);
		virtual Bound3f worldBound() = 0;
		virtual void needSample() {}; // ���ּ�������Ҫ���ɶ�����Ϣ�������������̣�ʹ�ø÷����������Ƿ���Ҫ��Щ��Ϣ
		virtual SurfaceRecord Sample(Sampler& sampler, Float& possibility) = 0; // �����Ϊ�������в���
		virtual SurfaceRecord Sample(const HitRecord& rec, Sampler& sampler, Float& possibility); // �������Ϊ�������в���
		virtual SamplerRequestInfo getSamplerRequest() {
			return SamplerRequestInfo();
		}
	protected:
		Transform toLocal, toWorld;
	};
}

