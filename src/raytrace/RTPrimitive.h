#pragma once
#include "mcl.h"
#include "types.h"
#include "Sampler.h"
namespace mcl {
	class RTPrimitive
	{
	public:
		virtual SamplerRequestInfo getSamplerRequest() const;

		virtual bool intersect(const Ray& ray, HitRecord* rec) = 0;
		/**
		 * @brief 用于快速阴影测试
		 */
		virtual bool intersectShadow(const Ray& ray) = 0;

		virtual Material* getMaterial() const {
			LOG(FATAL) << "Has no material!";
			return nullptr;
		}

		virtual std::unique_ptr<BsdfGroup> getBsdf(HitRecord* rec, Sampler& sampler) const {
			LOG(FATAL) << "Has no bsdf !";
			return nullptr;
		}

		virtual ~RTPrimitive() {};

		virtual RTSurfaceLight* getLight() const { return nullptr; }
		virtual std::shared_ptr<RTSurfaceLight> getSharedLight() const { return nullptr; }
	};

	class RTGeometryPrimitive:public RTPrimitive
	{
	public:
		RTGeometryPrimitive(std::shared_ptr<Geometry> geo, std::shared_ptr<Material> mat, std::shared_ptr<RTSurfaceLight> light = nullptr);

		Material* getMaterial() const  override;
		std::unique_ptr<BsdfGroup> getBsdf(HitRecord* rec, Sampler& sampler) const override;
		bool intersect(const Ray& ray, HitRecord* rec) override;
		bool intersectShadow(const Ray& ray) override;

		RTSurfaceLight* getLight() const override{
			return light.get();
		}
		std::shared_ptr<RTSurfaceLight> getSharedLight() const override{ 
			return light; 
		}

	private:
		std::shared_ptr<Geometry> geo;
		std::shared_ptr<Material> mat;
		std::shared_ptr<RTSurfaceLight> light;
	};
}

