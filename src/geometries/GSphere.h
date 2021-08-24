#pragma once
#include "mcl.h"
#include "types.h"
#include "Geometry.h"

namespace mcl {
	class GSphere: public Geometry
	{
	public:
		//����ı任��������и������Ե�scale�任
		GSphere(Point3f center, Float radius, const Transform& tran = Transform());
		virtual bool intersect(const Ray& ray, HitRecord* rec) override;
		//virtual bool intersectShadow(const Ray& ray) override; // #TODO2
		virtual Bound3f worldBound();
		virtual SurfaceRecord Sample(Sampler& sampler, Float& possibility) override; // �����Ϊ�������в���
		//#TODO1
		//virtual SurfaceRecord Sample(const HitRecord& rec, Sampler& sampler, Float& possibility) = 0; // �����Ϊ�������в���
		virtual SamplerRequestInfo getSamplerRequest() override {
			return SamplerRequestInfo(0,1);
		}

	private:
		void calRecord(const Point3f& pos,SurfaceRecord& rec);

		Float invArea;
	};
}

