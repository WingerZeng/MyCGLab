#pragma once
#include "mcl.h"
#include "types.h"

namespace mcl {

	class RayTracer
	{
	public:
		RayTracer(const std::shared_ptr<Film>& film)
			:film(film) {};

		virtual void render(const RTScene& scene) = 0;
		virtual void renderPixel(const RTScene& scene, const Point2i& pixel) = 0;
		virtual Color3f trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data) = 0;
		void setSampleNum(int num);
		void setThreadNum(int num);
		std::shared_ptr<mcl::Film> getFilm() const { return film; }

		virtual ~RayTracer() {};
	protected:
		const int MaxDepth = 4;
		int threadNum = 6;
		int SamplesPerPixelPerDim = 8;
		std::shared_ptr<Film> film;
	};

	class SampleRayTracer: public RayTracer 
	{ 
	public:
		SampleRayTracer(const std::shared_ptr<Film>& film, const std::shared_ptr<Sampler>& sampler)
			:RayTracer(film),sampler(sampler)
		{
		}

		void render(const RTScene& scene) override;
		void renderPixel(const RTScene& scene, const Point2i& pixel) override;
		Color3f trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data) override;
		//#TODO1 子类重载更高效的样本数初始化方法
		virtual void initializeSampler();

	protected:
		std::shared_ptr<Sampler> sampler;
	};

	class DirectLightPathTracer :public SampleRayTracer
	{
	public:
		using SampleRayTracer::SampleRayTracer;

		Color3f trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data) override;
		Color3f trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data, bool hitDiffuse);

	private:
		inline Float calRussianRoulettePossibility(Color3f color);

		const int MaxMaxDepth = 20; //超过MaxDepth则采用Russian Roulette，超过MaxMaxDepth则直接返回0
	};

	class MaterialTestTracer :public SampleRayTracer
	{
	public:
		using SampleRayTracer::SampleRayTracer;

		Color3f trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data) override;
	};

	Float DirectLightPathTracer::calRussianRoulettePossibility(Color3f color)
	{
		Float maxdim = color.max();
		return std::clamp(maxdim, Float(0.0), Float(1.0));
	}
}

