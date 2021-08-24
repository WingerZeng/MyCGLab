#include "Lambertain.h"
#include "Record.h"
#include "algorithms.h"
namespace mcl{
	
	LambertainBsdf::LambertainBsdf(const Color3f& factor, ScatterType sct)
		:Bsdf(Diffuse, sct),factor(factor)
	{
	}

	mcl::Color3f LambertainBsdf::calF(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct)) return Color3f(0);
		return factor * INVPI;
	}

	mcl::Color3f LambertainBsdf::calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct)) {
			possibility = 0;
			return Color3f(0);
		}
		//cos theta 重要性半球采样
		switch (activeScatterType(sct))
		{
		case Reflect:
			in = cosSampleHemisphere(sampler.get2DSample());
			in.z() *= out.z() > 0 ? 1: -1;
			break;
		case Refract:
			in = cosSampleHemisphere(sampler.get2DSample());
			in.z() *= out.z() > 0 ? -1 : 1;
			break;
		case Refract | Reflect:
			//暂时不支持, 启用后calF的返回值要减半
#if 1
			assert(0);
#else
			Point2f rand2d = sampler.get2DSample();
			bool reflect = rand2d.x() >= 0.5;
			rand2d.x() *= 10;
			rand2d.x() = rand2d.x() - std::floor(rand2d.x());
			assert(rand2d.x() <= 1 && rand2d.x() >= 0);
			in = cosSampleHemisphere(rand2d);
			if (reflect) {
				in.z() *= out.z() > 0 ? 1 : -1;
			}
			else {
				in.z() *= out.z() > 0 ? -1 : 1;
			}
#endif
		default:
			assert(0);
		}

#if 0
		if (scatterType() == Refract | Reflect) {
			possibility = calAbsCosTheta(in)*INVPI / 2;
			return factor * INVPI / 2;
		}
#endif

		possibility = calAbsCosTheta(in)*INVPI;
		return factor * INVPI;
	}

	Float LambertainBsdf::calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct)) {
			return 0;
		}
		switch (scatterType())
		{
		case Reflect:
			return calAbsCosTheta(in)*INVPI;
		case Refract:
			return calAbsCosTheta(in)*INVPI;
		case Refract | Reflect:
#if 1
			assert(0);
#else
			return calAbsCosTheta(in)*INVPI/2;
#endif
		default:
			assert(0);
			return 0;
		}
	}

	mcl::SamplerRequestInfo LambertainBsdf::getSamplerRequest() const
	{
		return SamplerRequestInfo(0, 1);
	}

	mcl::Color3f LambertainBsdf::albedo() const
	{
		return factor;
	}

	LambertainMaterial::LambertainMaterial(Color3f diffuse)
		:diffuse(std::make_shared<ConstantTexture<Color3f>>(diffuse))
	{

	}

	LambertainMaterial::LambertainMaterial(const std::shared_ptr<Texture<Color3f>>& diffuse)
		:diffuse(diffuse)
	{

	}

	LambertainMaterial::LambertainMaterial(DataNode* data)
	{
		diffuse = getColorTexture("Kd",data);
	}

	std::unique_ptr<mcl::BsdfGroup> LambertainMaterial::getBsdfs(HitRecord* rec, Sampler& sampler) const
	{
		assert(rec->n.normalized());
		bumpNormal(rec);
		std::unique_ptr<BsdfGroup> bsdfs(new BsdfGroup(Vector3f(rec->bumped_n), Normalize(rec->uvec), Vector3f(rec->n)));
		bsdfs->addBsdf(std::unique_ptr<Bsdf>(new LambertainBsdf(diffuse->value(*rec))));
		return bsdfs;
	}

}