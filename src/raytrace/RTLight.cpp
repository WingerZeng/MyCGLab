#include "RTLight.h"
#include "Geometry.h"
#include "algorithms.h"
namespace mcl{
	
	mcl::SamplerRequestInfo RTLight::getSamplerRequest() const
	{
		return SamplerRequestInfo();
	}

	RTSurfaceLight::RTSurfaceLight(const Color3f& e, std::shared_ptr<Geometry> geo, bool bothSides)
		:e(e),geo(geo),bothSides(bothSides)
	{
		geo->needSample();
	}

	mcl::SamplerRequestInfo RTSurfaceLight::getSamplerRequest() const
	{
		return geo->getSamplerRequest();
	}

	mcl::Color3f RTSurfaceLight::emission(const SurfaceRecord& rec, const Vector3f& vec) const
	{
		if (bothSides || vec.dot(Vector3f(rec.n)) > 0) return e;
		return Color3f(0);
	}

	mcl::Color3f RTSurfaceLight::sampleLight(const HitRecord& rec, Sampler& sampler, Point3f& pos, Float& possibility) const
	{
		SurfaceRecord srec = geo->Sample(rec, sampler, possibility);
		assert(srec.n.normalized());

		Vector3f lvec = srec.pos - rec.pos;
		pos = srec.pos;

		return emission(srec, -lvec);
	}

	RTSkyBox::RTSkyBox(std::shared_ptr<Texture<Color3f>> tex, const Color3f& le, const Transform& toWorld /*= Transform()*/)
		:tex(tex),le(le),toworld(toWorld),tolocal(toWorld.inverse())
	{

	}

	mcl::Color3f RTSkyBox::sampleLight(const HitRecord& rec, Sampler& sampler, Point3f& pos, Float& possibility) const
	{
		pos = uniformSampleSphere(sampler.get2DSample());
		Point2f uv = calUvInSphere(Vector3f(pos));
		pos = toworld(InvEpsilon * pos);
		possibility = 1.0 / 4 * INVPI;
		return le & tex->value(uv);
	}

	mcl::Color3f RTSkyBox::background(const Ray& ray) const
	{
		Vector3f rayd = tolocal(ray.d);
		Point2f uv = calUvInSphere(rayd);
		return le & tex->value(uv);
	}

}