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
		virtual void needSample() {}; // 部分几何体需要生成额外信息来辅助采样过程，使用该方法来控制是否需要这些信息
		virtual SurfaceRecord Sample(Sampler& sampler, Float& possibility) = 0; // 以面积为度量进行采样
		virtual SurfaceRecord Sample(const HitRecord& rec, Sampler& sampler, Float& possibility); // 以立体角为度量进行采样
		virtual SamplerRequestInfo getSamplerRequest() {
			return SamplerRequestInfo();
		}
	protected:
		Transform toLocal, toWorld;
	};
}

