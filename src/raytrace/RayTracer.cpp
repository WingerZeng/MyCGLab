#include <omp.h>
#include <queue>
#include "RayTracer.h"
#include "Sampler.h"
#include "RTScene.h"
#include "RTCamera.h"
#include "Film.h"
#include "Record.h"
#include "RTPrimitive.h"
#include "RTLight.h"
#include "Material.h"
#include "Geometry.h"
#include "Bsdf.h"
#include "algorithms.h"
#include "ui/Console.h"
//#TEST
#include "utilities.h"

//#define PRINTTRACEINFO

namespace mcl{
	
	void SampleRayTracer::render(const RTScene& scene)
	{
		//初始化
		initializeSampler();
		Point2i size = film->getSize();
		const int pixelCount = size.x() * size.y();
		const int sampleCount = sampler->samplePerPixel().x() * sampler->samplePerPixel().y();
		
		int nthreads, tid;
		std::unique_ptr<int> count(new int(0));
		omp_set_num_threads(threadNum);
		std::queue<QString> textQue;
#pragma omp parallel private(nthreads,tid) num_threads(threadNum)
		{
			auto mysampler = sampler->clone();
			tid = omp_get_thread_num();
			nthreads = omp_get_num_threads();

			while (*count != pixelCount) {
				int thiscount;
				#pragma omp critical
				{
					thiscount = (*count)++;
				}
				if(*count >= pixelCount) break;

				int y = thiscount / size.x();
				int x = thiscount - y * size.x();

				//计算单个像素
				Point2i xy(x, y);

				mysampler->startPixel(xy);

				Color3f result;
				PixelData data;
				for (int isample = 0; isample < sampleCount; isample++) {
					Ray ray = scene.getCamera()->getRay(*mysampler, *film, xy);
					PixelData tempdata;
					result += trace(ray, scene, *mysampler, tempdata);
					data += tempdata;
					mysampler->toNextSample();
				}
				data /= sampleCount;
				data = scene.getCamera()->worldToView()(data);
				result /= sampleCount;
				film->setPixelData(xy, data);
				film->setPixelColor(xy, result);

				if (thiscount % (size.x()) == 0) {
					textQue.push("finish  " + QString::number(y * 100.0 / size.y()) + "%");
				}
				while (!tid && !textQue.empty()) {
					EXEPTR->printMessage(textQue.front(), QConsole::Partial);
					textQue.pop();
				}
			}
		}
	}

	void SampleRayTracer::renderPixel(const RTScene& scene, const Point2i& pixel)
	{
		initializeSampler();
		Point2i size = film->getSize();
		const int sampleCount = sampler->samplePerPixel().x() * sampler->samplePerPixel().y();

		auto mysampler = sampler->clone();

		//计算单个像素
		mysampler->startPixel(pixel);
		Color3f result;
		PixelData data;
		for (int isample = 0; isample < sampleCount; isample++) {
			Ray ray = scene.getCamera()->getRay(*mysampler, *film, pixel);
			PixelData tempdata;
			Color3f tempresult = trace(ray, scene, *mysampler, data);
			PRINTINFO(isample);
			PRINTINFO(tempresult);
			result += tempresult;
			data += tempdata;
			mysampler->toNextSample();
		}
		result /= sampleCount;
		data /= sampleCount;
		film->setPixelData(pixel, data);
		film->setPixelColor(pixel, result);
	}

	mcl::Color3f SampleRayTracer::trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data)
	{
		//#TODO6 该算法的data值尚未维护
		if (ray.depth > MaxDepth) return Color3f(0,0,0);
		HitRecord rec;
		Color3f result;

		if (scene.intersect(ray, &rec)) {
			Point3f hitp = ray.pos(rec.t);


			auto selflight = rec.prim->getLight();
			if (selflight) {
				result += selflight->emission(rec,-ray.d);
			}

			std::unique_ptr<Bsdf> bsdf = rec.prim->getBsdf(&rec, sampler);
			if (!bsdf) return result;
			if (bsdf->hasType(Diffuse, ALL_Scatter)) {
				for (const auto& light : scene.getLights()) {
					if (selflight == light.get()) continue;
					Float possibility;
					Point3f pos;
					Color3f emission = light->sampleLight(rec, sampler, pos, possibility);

					Vector3f vec = (pos - hitp);

					Float dist = vec.length();
					vec.normalize();

					//这里可能会和光源模型本身相交，因此偏移一个ZERO
					Ray shadowray(hitp - RayTraceZero * ray.d, vec, 0, dist - RayTraceZero, ray.time, 0);
					Color3f diffuse = bsdf->calF(-ray.d, vec);

					if (!isDark(diffuse) && !scene.intersectShadow(shadowray)) {
						result += diffuse & emission / possibility * std::clamp(vec.absDot(Vector3f(rec.bumped_n)), 0.0f, 1.0f);
					}
				}
			}

			//追踪反射光线
			if (bsdf->hasType(Specular, Reflect)) {
				Vector3f invec;
				Float possibility;
				Color3f f = bsdf->calIn(-ray.d,invec, possibility,sampler,Specular,Reflect);
				if (possibility && !isDark(f)) {
					Ray newray(hitp - RayTraceZero * ray.d, invec, 0, Infinity, ray.time, ray.depth + 1);
					result += f & trace(newray, scene, sampler, data) * invec.absDot(Vector3f(rec.bumped_n)) / possibility;
				}
			}

			//追踪折射光线
			if (bsdf->hasType(Specular, Refract)) {
				Vector3f invec;
				Float possibility;
				Color3f f = bsdf->calIn(-ray.d, invec, possibility, sampler, Specular, Refract);
				if (possibility && !isDark(f)) {
					Ray newray(hitp + RayTraceZero * ray.d, invec, 0, Infinity, ray.time, ray.depth + 1);
					result += f & trace(newray, scene, sampler, data) * invec.absDot(Vector3f(rec.bumped_n)) / possibility;
				}
			}
			
			return result;
		}
		else {
			for (const auto& light : scene.getLights()) {
				result += light->background(ray);
			}
			return result;
		}
	}

	void SampleRayTracer::initializeSampler()
	{
		sampler->setRequest(SamplerRequestInfo(20 * MaxDepth, 20 * MaxDepth, Point2i(SamplesPerPixelPerDim, SamplesPerPixelPerDim)));
	}

	mcl::Color3f DirectLightPathTracer::trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data)
	{
		return trace(ray, scene, sampler, data, false);
	}

	mcl::Color3f DirectLightPathTracer::trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data, bool hitDiffuse)
	{
		HitRecord rec;
		Color3f result;

		auto isContinueTrace = [&](const Color3f& color, Float& p){
			p = 1;
			if (ray.depth > MaxDepth) {
				if (ray.depth > MaxMaxDepth) return false;
				else {
					p = calRussianRoulettePossibility(color);
					Float sample = sampler.get1DSample();
					return sample < p;
				}
			}
			return true;
		};

		PixelData diffData, reflData, refrData;
		Color3f diffColor, reflColor, refrColor;
		Color3f diffFactor, reflFactor, refrFactor;

		if (scene.intersect(ray, &rec)) {
			Point3f hitp = ray.pos(rec.t);

			auto selflight = rec.prim->getLight();

			//只有当之前没有计算过直接光照，也就是没有经过diffuse材质时，才计入击中光源的光照
			if (selflight && !hitDiffuse) {
				result += selflight->emission(rec, -ray.d);
				data.albedo += result;
			}

			std::unique_ptr<Bsdf> bsdf = rec.prim->getBsdf(&rec, sampler);
			if (!bsdf) {
				data.normal = rec.bumped_n;
				return result;
			}

			// 出射点偏移
			Vector3f posOffset = rec.pos.length() * RayTraceZero * ray.d / 100;

			if (bsdf->hasType(Diffuse, ALL_Scatter)) {
				//计算直接光照
				for (const auto& light : scene.getLights()) {
					if (selflight == light.get()) continue;
					Float possibility;
					Point3f pos;
					Color3f emission = light->sampleLight(rec, sampler, pos, possibility);

					Vector3f vec = (pos - hitp);

					Float dist = vec.length();
					vec.normalize();

					//这里可能会和光源模型本身相交，因此偏移一个ZERO
					Ray shadowray(hitp - posOffset, vec, 0, dist - RayTraceZero, ray.time, 0);
					Color3f diffuse = bsdf->calF(-ray.d, vec, Diffuse, ALL_Scatter);
					diffFactor += diffuse;
					if (!isDark(diffuse) && !scene.intersectShadow(shadowray)) {
						result += diffuse & emission / possibility * std::clamp(vec.absDot(Vector3f(rec.bumped_n)), 0.0f, 1.0f);
					}
				}
				diffFactor /= scene.getLights().size();
#ifdef PRINTTRACEINFO
				std::cout << "Depth" << ray.depth << ":DirectLight " << result << std::endl;
#endif
				//继续追踪间接光照
				Vector3f invec;
				Float possibility;
				Color3f f = bsdf->calIn(-ray.d, invec, possibility, sampler, Diffuse, ALL_Scatter);

				if (possibility && !isDark(f)) {
					Color3f factor = f * invec.absDot(Vector3f(rec.bumped_n)) / possibility;
					Float p;
					if (isContinueTrace(factor,p)) {
#ifdef PRINTTRACEINFO
						std::cout << "Depth" << ray.depth << ":DiffuseFactor " << factor << std::endl;
						std::cout << "Go Diffuse Depth" << ray.depth + 1 << std::endl;
#endif
						diffFactor = (diffFactor + factor / p) / 2;
						Ray newray(hitp - posOffset, invec, 0, Infinity, ray.time, ray.depth + 1);
						Color3f tempresult = factor & trace(newray, scene, sampler, diffData, true) / p;
						result += tempresult;
#ifdef PRINTTRACEINFO
						std::cout << "Diffuse From Depth" << ray.depth + 1 << " " << tempresult << std::endl;
#endif
					}
				}

			}
			diffColor = result;

			//追踪反射光线
			if (bsdf->hasType(Specular, Reflect)) {
				Vector3f invec;
				Float possibility;
				Color3f f = bsdf->calIn(-ray.d, invec, possibility, sampler, Specular, Reflect);

				if (possibility && !isDark(f)) {
					Color3f factor = f * invec.absDot(Vector3f(rec.bumped_n)) / possibility;
					Float p;
					if (isContinueTrace(reflFactor,p)) {
#ifdef PRINTTRACEINFO
						std::cout << "Depth" << ray.depth << ":ReflectFactor " << f * invec.absDot(Vector3f(rec.bumped_n)) / possibility << std::endl;
						std::cout << "Go Reflect Depth" << ray.depth + 1 << std::endl;
#endif
						reflFactor = factor / p;
						Ray newray(hitp - posOffset, invec, 0, Infinity, ray.time, ray.depth + 1);
						reflColor = reflFactor & trace(newray, scene, sampler, reflData, false);
						result += reflColor;
#ifdef PRINTTRACEINFO
						std::cout << "Reflect From Depth" << ray.depth + 1 << " " << tempresult << std::endl;
#endif
					}
				}
			}

			//追踪折射光线
			if (bsdf->hasType(Specular, Refract)) {
				Vector3f invec;
				Float possibility;
				Color3f f = bsdf->calIn(-ray.d, invec, possibility, sampler, Specular, Refract);

				if (possibility && !isDark(f)) {
					Color3f factor = f * invec.absDot(Vector3f(rec.bumped_n)) / possibility;
					Float p;
					if (isContinueTrace(factor, p)) {
#ifdef PRINTTRACEINFO
						std::cout << "Depth" << ray.depth << ":RefractFactor " << f * invec.absDot(Vector3f(rec.bumped_n)) / possibility << std::endl;
						std::cout << "Go Refract Depth" << ray.depth << std::endl;
#endif
						refrFactor = factor / p;
						Ray newray(hitp + posOffset, invec, 0, Infinity, ray.time, ray.depth + 1);
						refrColor = refrFactor & trace(newray, scene, sampler, refrData, false);
						result += refrColor;
#ifdef PRINTTRACEINFO
						std::cout << "Reflect From Depth" << ray.depth + 1 << " " << tempresult << std::endl;
#endif
					}
				}
			}
#ifdef PRINTTRACEINFO
			std::cout << "Total Result Depth" << ray.depth << " " << result << std::endl;
#endif
			Color3f totFactor = diffFactor + refrFactor + reflFactor;
			if (/*totFactor.length() > FloatZero*/0) {
				//Color3f invTotFactor = totFactor.inverse();
				//Color3f diffFactorV = diffFactor & invTotFactor;
				//Color3f reflFactorV = reflFactor & invTotFactor;
				//Color3f refrFactorV = refrFactor & invTotFactor;
				//Float diffFactorS = diffFactor.length();
				//Float reflFactorS = reflFactor.length();
				//Float refrFactorS = refrFactor.length();
				//Float invTotFactorS = 1 / (diffFactorS + reflFactorS + refrFactorS);
				//diffFactorS *= invTotFactorS;
				//reflFactorS *= invTotFactorS;
				//refrFactorS *= invTotFactorS;
				//data.normal = Normalize(diffFactorS * rec.bumped_n + reflFactorS * reflData.normal + refrFactorS * refrData.normal);
				//data.albedo = (diffFactorS * bsdf->albedo() + reflFactorS * reflData.albedo + refrFactorS * refrData.albedo);
				bool hasDiff = diffFactor.length() > 0;
				bool hasRefl = reflFactor.length() > 0;
				bool hasRefr = refrFactor.length() > 0;
				int cnt = 0;
				Normal3f normal;
				Color3f albedo;
				if (hasDiff) {
					cnt++;
					normal += rec.bumped_n;
					albedo += bsdf->albedo().length() ? bsdf->albedo() : Color3f();
				}
				if (hasRefl) {
					cnt++;
					normal += reflData.normal;
					albedo += reflData.albedo.length() ? reflData.albedo : Color3f();
				}
				if (hasRefr) {
					cnt++;
					normal += refrData.normal;
					albedo += refrData.albedo.length() ? refrData.albedo : Color3f();
				}
				data.normal = Normalize(normal / cnt);
				data.albedo = albedo / cnt;
			}
			else {
				//目前的额外信息只取决于第一次hit的值
				data.albedo += bsdf->albedo();
				data.normal = rec.bumped_n;
			}
			return result;
		}
		else {
			if (hitDiffuse) {
				return Color3f(0, 0, 0);
			}
			for (const auto& light : scene.getLights()) {
				result += light->background(ray);
			}
#ifdef PRINTTRACEINFO
			std::cout << "Background Depth" << ray.depth << " " << result << std::endl;
#endif
			data.albedo = result;
			data.normal = Normal3f(uniformSampleSphere(sampler.get2DSample()));
			return result;
		}
	}

	mcl::Color3f MaterialTestTracer::trace(const Ray& ray, const RTScene& scene, Sampler& sampler, PixelData& data)
	{
		HitRecord rec;
		Color3f result;

		if (scene.intersect(ray, &rec)) {
			return Color3f(rec.uv[0], rec.uv[1], 0);


			Point3f hitp = ray.pos(rec.t);

			auto selflight = rec.prim->getLight();

			std::unique_ptr<Bsdf> bsdf = rec.prim->getBsdf(&rec, sampler);
			if (!bsdf) return result;


			//追踪反射光线
			if (bsdf->hasType(Specular, Reflect)) {
				Vector3f invec;
				Float possibility;
				Color3f f = bsdf->calIn(-ray.d, invec, possibility, sampler, Specular, Reflect);
				if (invec.length()) {
					invec.normalize();
					result = Color3f(invec.x(), invec.y(), invec.z());
				}
			}

			return result;
		}
		else {
			return Color3f(0,0,0);
		}
	}

	void RayTracer::setSampleNum(int num)
	{
		if (num > 0)
			SamplesPerPixelPerDim = num;
	}

	void RayTracer::setThreadNum(int num)
	{
		if (num > 0)
			threadNum = num;
	}

}