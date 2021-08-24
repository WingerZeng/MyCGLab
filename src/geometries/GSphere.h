#pragma once
#include "mcl.h"
#include "types.h"
#include "Geometry.h"

namespace mcl {
	class GSphere: public Geometry
	{
	public:
		//传入的变换不允许带有各向异性的scale变换
		GSphere(Point3f center, Float radius, const Transform& tran = Transform());
		virtual bool intersect(const Ray& ray, HitRecord* rec) override;
		//virtual bool intersectShadow(const Ray& ray) override; // #TODO2
		virtual Bound3f worldBound();
		virtual SurfaceRecord Sample(Sampler& sampler, Float& possibility) override; // 以面积为度量进行采样
		//#TODO1
		//virtual SurfaceRecord Sample(const HitRecord& rec, Sampler& sampler, Float& possibility) = 0; // 以面积为度量进行采样
		virtual SamplerRequestInfo getSamplerRequest() override {
			return SamplerRequestInfo(0,1);
		}

	private:
		void calRecord(const Point3f& pos,SurfaceRecord& rec);

		Float invArea;
	};
}

