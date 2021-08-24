#pragma once
#include "mcl.h"
#include "types.h"
#include "Sampler.h"
#include "Record.h"
#include "Texture.h"
namespace mcl {
	class RTLight
	{
	public:
		virtual SamplerRequestInfo getSamplerRequest() const;

		//#TODO2 可见性测试要单独封装一个类
		virtual Color3f sampleLight(const HitRecord& rec, Sampler& sampler,Point3f& pos,Float& possibility) const = 0;

		virtual Color3f background(const Ray& ray) const {
			return Color3f(0); 
		}

		virtual ~RTLight() {};
	};

	class RTPointLight:public RTLight
	{
	public:
		RTPointLight(const Color3f& e, const Point3f& pos) 
		:e(e),pos(pos){

		}

		virtual Color3f sampleLight(const HitRecord& rec, Sampler& sampler, Point3f& apos, Float& possibility) const override {
			apos = pos;
			possibility = 1;
			return e;
		}

	private:
		Color3f e;
		Point3f pos;
	};

	class RTSurfaceLight :public RTLight
	{
	public:
		RTSurfaceLight(const Color3f& e, std::shared_ptr<Geometry> geo, bool bothSides = true);

		virtual SamplerRequestInfo getSamplerRequest() const override;

		/**
		 * @brief 用于光线直接命中光源时获取光源辐射度，vec方向由表面向外
		 */
		Color3f emission(const SurfaceRecord& rec, const Vector3f& vec) const;

		Color3f background(const Ray& ray) const override { return bgc; }
		void setBackground(const Color3f& bg) { bgc = bg; }

		virtual Color3f sampleLight(const HitRecord& rec, Sampler& sampler, Point3f& pos, Float& possibility) const override;

		Geometry* getGeometry() { return geo.get(); }
		std::shared_ptr<Geometry> getSharedGeometry() { return geo; }
	private:
		Color3f e;
		Color3f bgc;
		std::shared_ptr<Geometry> geo;
		bool bothSides;
	};

	class RTSkyBox : public RTLight
	{
	public:
		RTSkyBox(std::shared_ptr<Texture<Color3f>> tex, const Color3f& le, const Transform& toWorld = Transform());

		virtual Color3f sampleLight(const HitRecord& rec, Sampler& sampler, Point3f& pos, Float& possibility) const override;

		virtual Color3f background(const Ray& ray) const override;

	private:
		std::shared_ptr<Texture<Color3f>> tex;
		const Color3f le;
		const Transform toworld;
		const Transform tolocal;
	};
}

