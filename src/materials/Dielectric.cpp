#include "Dielectric.h"
#include "Record.h"
#include "algorithms.h"
namespace mcl{
	
	DielectricBsdf::DielectricBsdf(const Color3f& reflFactor, const Color3f& refrFactor, Float eta1, Float eta2)
		:Bsdf(Specular,ScatterType(Refract|Reflect)),reflFactor(reflFactor),refrFactor(refrFactor),eta1(eta1),eta2(eta2)
	{
	}

	mcl::Color3f DielectricBsdf::calF(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct));
		{
			return Color3f(0);
		}

		if (activeScatterType(Reflect) && isMirror(out, in)) {
			//calculate Fresnel coefficient
			Float etaout = out.z() > 0 ? eta1 : eta2;
			Float etain = in.z() > 0 ? eta1 : eta2;
			Float cosout = calAbsCosTheta(out);
			Float cosin = calAbsCosTheta(in);

			return reflFactor * calReflectFresnel(cosout, cosin, etaout, etain) * calCosPhi(in);
		}

		if (activeScatterType(Refract)&&isRefract(out, in, eta1, eta2)) {
			//calculate Fresnel coefficient
			Float etaout = out.z() > 0 ? eta1 : eta2;
			Float etain = in.z() > 0 ? eta1 : eta2;
			Float cosout = calAbsCosTheta(out);
			Float cosin = calAbsCosTheta(in);

			return refrFactor * (1-calReflectFresnel(cosout,cosin,etaout,etain)) * calCosPhi(in) * etaout * etaout / (etain * etain);
		}
		return Color3f(0);
	}

	mcl::Color3f DielectricBsdf::calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct))
		{
			possibility = 0;
			return Color3f(0);
		}

		ScatterType con = NULL_Scatter;

		bool into = out.z() > 0;

		Float etaout = into ? eta1 : eta2;
		Float etain = into ? eta2 : eta1;
		Float invetain = 1 / etain;
		Float eta_ = etaout * etaout * invetain * invetain;

		Float outCosTheta = calAbsCosTheta(out);
		Float outSinTheta2 = 1 - outCosTheta * outCosTheta;
		Float inSinTheta2 = outSinTheta2 * eta_;
		Float inCosTheta = std::sqrt(1-inSinTheta2);

		if ((sct & Refract) && inSinTheta2 < 1) {
			con = ScatterType(con | Refract);
		}
		
		if (sct & Reflect) {
			con = ScatterType(con | Reflect);
		}

		Float cRefl = calReflectFresnel(outCosTheta, inCosTheta, etaout, etain);
		Float p;

		switch (con)
		{
		case NULL_Scatter:
			possibility = 0;
			return Color3f(0);
			break;

		case Reflect:
			in = Vector3f(-out.x(), -out.y(), out.z());
			possibility = 1;
			if (inSinTheta2 < 1) {
				return reflFactor * cRefl / outCosTheta;
			}
			else {
				return reflFactor * 1 / outCosTheta; //全反射
			}
			break;

		case Refract:
			in = Vector3f(-out.x() * etaout * invetain, -out.y() * etaout * invetain, into ? -inCosTheta : inCosTheta);
			possibility = 1;
			return eta_ * refrFactor * (1- cRefl) / inCosTheta;
			break;

		case Refract | Reflect:  //两种情况都有时，随机选择其一
			p = sampler.get1DSample();
			if (inSinTheta2 > 1) { //全反射
				in = Vector3f(-out.x(), -out.y(), out.z());
				possibility = 1;
				return reflFactor * 1 / outCosTheta; //全反射
			}
			else {
				if (p < cRefl) { //选择反射光线
					in = Vector3f(-out.x(), -out.y(), out.z());
					possibility = cRefl;
					return reflFactor * cRefl / outCosTheta;
				}
				else { //选择折射光线
					in = Vector3f(-out.x() * etaout * invetain, -out.y() * etaout * invetain, into ? -inCosTheta : inCosTheta);
					possibility = 1 - cRefl;
					return eta_ * refrFactor * (1 - cRefl) / inCosTheta;
				}
			}
			break;

		default:
			assert(0);
			return Color3f(0);
			break;
		}
	}

	Float DielectricBsdf::calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct));
		{
			return 0;
		}

		if (isMirror(out, in)) {
			//#TODO2 //要区分全反射和折射、以及sct参数的情况，来计算概率
			assert(0);
		}
		if (isRefract(out, in, eta1, eta2)) {
			//#TODO2
			assert(0);
		}
		return 0;
	}

	Float DielectricBsdf::calReflectFresnel(const Float& outCosTheta, const Float& inCosTheta, const Float& etaout, const Float& etain) const
	{
		Float rh = (etaout * inCosTheta - etain * outCosTheta) / (etaout * inCosTheta + etain * outCosTheta);
		Float rv = (etaout * outCosTheta - etain * inCosTheta) / (etaout * outCosTheta + etain * inCosTheta);
		return (rh*rh + rv * rv) / 2;
	}

	DielectricMaterial::DielectricMaterial(Float eta1, Float eta2, const Color3f& aReflFactor /*= Color3f(1)*/, const Color3f& aRefrFactor /*= Color3f(1)*/)
		:reflFactor(std::make_shared<ConstantTexture<Color3f>>(aReflFactor)), refrFactor(std::make_shared<ConstantTexture<Color3f>>(aRefrFactor)), eta1(eta1), eta2(eta2)
	{

	}

	DielectricMaterial::DielectricMaterial(Float eta1, Float eta2, const std::shared_ptr<Texture<Color3f>>& reflFactor, const std::shared_ptr<Texture<Color3f>>& refrFactor)
		:reflFactor(reflFactor), refrFactor(refrFactor), eta1(eta1), eta2(eta2)
	{

	}

	DielectricMaterial::DielectricMaterial(DataNode* data)
	{
		reflFactor = getColorTexture("Kr", data);
		refrFactor= getColorTexture("Kt", data);
		if (data->fd("Eta1")) {
			eta1 = data->fd("Eta1")->toDouble();
		}
		else {
			eta1 = 1;
		}

		if (data->fd("Eta2")) {
			eta2 = data->fd("Eta2")->toDouble();
		}
		else {
			eta2 = 1.5;
		}
	}

	std::unique_ptr<mcl::BsdfGroup> DielectricMaterial::getBsdfs(HitRecord* rec, Sampler& sampler) const
	{
		assert(rec->n.normalized());
		bumpNormal(rec);
		std::unique_ptr<BsdfGroup> bsdfs(new BsdfGroup(Vector3f(rec->bumped_n), Normalize(rec->uvec), Vector3f(rec->n)));
		bsdfs->addBsdf(std::make_unique<DielectricBsdf>(reflFactor->value(*rec),refrFactor->value(*rec),eta1, eta2));
		return bsdfs;
	}

}