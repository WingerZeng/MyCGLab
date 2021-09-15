#include "Disney.h"
#include "algorithms.h"
namespace mcl{

	DisneyMaterial::DisneyMaterial(const DisneyMaterialInfo& info)
		:info(info)
	{

	}

	DisneyMaterial::DisneyMaterial(DataNode* data)
	{
		info.baseColor = getColorTexture("BaseColor",data);
		info.metallic = getFloatTexture("metallic", data);
		info.subsurface = getFloatTexture("subsurface", data);
		info.specular = getFloatTexture("specular", data);
		info.specularTint= getFloatTexture("specularTint", data);
		info.roughness= getFloatTexture("roughness", data);
		info.anisotropic = getFloatTexture("anisotropic", data);
		info.sheen = getFloatTexture("sheen", data);
		info.sheenTint= getFloatTexture("sheenTint", data);
		info.clearcoat= getFloatTexture("clearcoat", data);
		info.clearcoatGloss = getFloatTexture("clearcoatGloss", data);
	}

	std::unique_ptr<mcl::BsdfGroup> DisneyMaterial::getBsdfs(HitRecord* rec, Sampler& sampler) const
	{
		assert(rec->n.normalized());
		bumpNormal(rec);
		std::unique_ptr<BsdfGroup> bsdfs(new BsdfGroup(Vector3f(rec->bumped_n), Normalize(rec->uvec), Vector3f(rec->n)));
		bsdfs->addBsdf(std::make_unique<DisneyBrdf>(
			info.baseColor ? info.baseColor->value(*rec) : Color3f(1, 1, 1),
			info.metallic ? info.metallic->value(*rec) : 0,
			info.subsurface ? info.subsurface->value(*rec) : 0.5,
			info.specular ? info.specular->value(*rec) : 0,
			info.specularTint ? info.specularTint->value(*rec) : 0,
			info.roughness ? info.roughness->value(*rec) : 0.5,
			info.anisotropic ? info.anisotropic->value(*rec) : 0,
			info.sheen ? info.sheen->value(*rec) : 0,
			info.sheenTint ? info.sheenTint->value(*rec) : 0,
			info.clearcoat ? info.clearcoat->value(*rec) : 0,
			info.clearcoatGloss ? info.clearcoatGloss->value(*rec) : 0.5
		));
		return bsdfs;
	}

	DisneyBrdf::DisneyBrdf(const Color3f& baseColor, const Float& metallic, const Float& subsurface, const Float& specular, const Float& specularTint, const Float& roughness, const Float& anisotropic, const Float& sheen, const Float& sheenTint, const Float& clearcoat, const Float& clearcoatGloss)
		:Bsdf(ALL_Surface,Reflect),baseColor(limitColor(baseColor)),metallic(metallic),subsurface(subsurface),specular(specular),
		specularTint(specularTint),roughness(roughness), anisotropic(anisotropic),sheen(sheen),
		sheenTint(sheenTint), clearcoat(clearcoat), clearcoatGloss(clearcoatGloss),
		a(anisotropic ? sqrt(1 - 0.9*anisotropic) : 1),alphaX(roughness * roughness / a),alphaY(roughness * roughness * a),pclearcoat((clearcoat/4)/(clearcoat/4+specular)),
		cR0(interpolate(0.08 * specular * interpolate(Color3f(1, 1, 1), colorTint, specularTint), baseColor, metallic)),
		colorTint(lum(baseColor)),
		alpha_c(interpolate(0.01f, 0.1f, 1 - clearcoatGloss)*interpolate(0.01f, 0.1f, 1 - clearcoatGloss))
	{
		if (!clearcoat && !specular && !metallic) {
			sft = Diffuse;
		}
	}


	mcl::Color3f DisneyBrdf::calF(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct))
		{
			return Color3f(0);
		}
		
		ScatterInfo info;

		info.cosThetaIn = calAbsCosTheta(in);
		info.cosThetaOut = calAbsCosTheta(out);

		info.h = calHalfVec(in, out);
		info.cosThetaD = in.dot(info.h);

		info.cosThetaD_m1_5 = pow5(1 - info.cosThetaD);

		Color3f result;

		if (activeSurfaceType(sft) & Diffuse) {
			result = result + calDiffuse(out, in, info);
		}

		if (activeSurfaceType(sft) & Specular) {
			if (in.z() == 1) {
				info.cosPhiIn = 1 / SQRT2;
			}
			else {
				info.cosPhiIn = calCosPhi(in);
			}
			if (out.z() == 1) {
				info.cosPhiOut = 1 / SQRT2;
			}
			else {
				info.cosPhiOut = calCosPhi(out);
			}
			info.sinPhiIn = sqrt(1 - info.cosPhiIn * info.cosPhiIn);
			info.sinPhiOut = sqrt(1 - info.cosPhiOut * info.cosPhiOut);
			info.tanThetaIn = calAbsTanTheta(in);
			info.tanThetaOut = calAbsTanTheta(out);
			info.cosThetaH = calAbsCosTheta(info.h);
			info.sinThetaH = sqrt(1 - info.cosThetaH * info.cosThetaH);
			info.tsSpecularTerm = 1.0 / 4 / info.cosThetaIn / info.cosThetaOut;

			if (info.sinThetaH <= FloatZero) {
				info.PHI = 1;
			}
			else {
				const Float cosPhiH = calCosPhi(info.h);
				const Float sinPhiH = sqrt(1 - cosPhiH * cosPhiH);
				info.PHI = (cosPhiH * cosPhiH / (alphaX * alphaX) + sinPhiH * sinPhiH / (alphaY * alphaY));
			}

			result = result + (specular ? calSpecular(out, in, info) : Color3f(0)) + (clearcoat ? calClearcoat(out, in, info) : Color3f(0));
		}
		return result;
	}

	mcl::Color3f DisneyBrdf::calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct))
		{
			return Color3f(0);
		}
		bool calSpecular = activeSurfaceType(sft) & Specular;
		bool calDiffuse = activeSurfaceType(sft) & Diffuse;
		Point2f sample = sampler.get2DSample();
		if (calDiffuse && !calSpecular) {
			in = cosSampleHemisphere(sample);
			in.z() *= out.z() > 0 ? 1 : -1;
			possibility = calAbsCosTheta(in)*INVPI;
			return calF(out, in, Diffuse, Reflect);
		}
		else if(!calDiffuse && calSpecular) {
			const Float specualrSample = sampler.get1DSample();
			ScatterInfo info;
			if (specualrSample < pclearcoat) {
				/* Sample clearcoat term */
				// 1. Sample in vector
				const Float phiH = 2 * PI*sample.x();
				const Float cosPhiH = cos(phiH);
				const Float sinPhiH = sin(phiH);
				info.cosThetaH = sqrt((pow(alpha_c, sample.y()) - 1) / (alpha_c - 1));
				info.sinThetaH = sqrt(1 - info.cosThetaH * info.cosThetaH);				
				info.h = Vector3f(info.sinThetaH * cosPhiH, info.sinThetaH * sinPhiH, info.cosThetaH);
				info.h.z() *= out.z() > 0 ? 1 : -1;
				in = reflectByNormal(out, info.h);
				if (in.z() * out.z() <= 0) {
					possibility = 0;
					return Color3f(0, 0, 0);
				}

				// 2. Prepare information
				info.cosThetaIn = calAbsCosTheta(in);
				info.cosThetaOut = calAbsCosTheta(out);
				info.cosThetaD = in.dot(info.h);
				info.cosThetaD_m1_5 = pow5(1 - info.cosThetaD);
				info.tanThetaIn = calAbsTanTheta(in);
				info.tanThetaOut = calAbsTanTheta(out);
				info.tsSpecularTerm = 1.0 / 4 / info.cosThetaIn / info.cosThetaOut;


				// 3. Calculate factor
				Color3f result = calClearcoat(out, in, info);
				possibility = info.Dc * info.cosThetaH / 4 / info.cosThetaD * pclearcoat;
				return result;
			}
			else {
				/* Sample specular term */
				// 1. Sample in vector
				const Float cosSample1 = cos(2 * PI*sample.x());
				const Float sinSample1 = sin(2 * PI*sample.x());
				Float cosPhiH;
				Float sinPhiH;

				if (alphaX < FloatZero) {
					cosPhiH = sinSample1;
					sinPhiH = cosSample1;
					info.cosThetaH = 1;
					info.sinThetaH = 0;
				}
				else {
					Float r = sqrt(alphaY*alphaY*sinSample1*sinSample1 + alphaX * alphaX*cosSample1*cosSample1);
					cosPhiH = alphaY / r * sinSample1;
					sinPhiH = alphaX / r * cosSample1;
					info.PHI = (cosPhiH * cosPhiH / (alphaX * alphaX) + sinPhiH * sinPhiH / (alphaY * alphaY));
					info.cosThetaH = std::sqrt(info.PHI * (1 - sample.y()) / ((1 - info.PHI)*sample.y() + info.PHI));
					info.sinThetaH = sqrt(1 - info.cosThetaH * info.cosThetaH);
				}
				if (info.sinThetaH <= FloatZero) {
					info.PHI = 1;
				}


				info.h = Vector3f(info.sinThetaH * cosPhiH, info.sinThetaH * sinPhiH, info.cosThetaH);
				info.h.z() *= out.z() > 0 ? 1 : -1;

				in = reflectByNormal(out, info.h);
				if (in.z() * out.z() <= 0) {
					possibility = 0;
					return Color3f(0, 0, 0);
				}

				// 2. Prepare information
				info.cosThetaIn = calAbsCosTheta(in);
				info.cosThetaOut = calAbsCosTheta(out);

				info.cosThetaD = in.dot(info.h);
				info.cosThetaD_m1_5 = pow5(1 - info.cosThetaD);
				
				if (in.z() == 1) {
					info.cosPhiIn = 1 / SQRT2;
				}
				else {
					info.cosPhiIn = calCosPhi(in);
				}
				if (out.z() == 1) {
					info.cosPhiOut = 1 / SQRT2;
				}
				else {
					info.cosPhiOut = calCosPhi(out);
				}
				info.sinPhiIn = sqrt(1 - info.cosPhiIn * info.cosPhiIn);
				info.sinPhiOut = sqrt(1 - info.cosPhiOut * info.cosPhiOut);
				info.tanThetaIn = calAbsTanTheta(in);
				info.tanThetaOut = calAbsTanTheta(out);
				info.cosThetaH = calAbsCosTheta(info.h);
				info.sinThetaH = sqrt(1 - info.cosThetaH * info.cosThetaH);
				info.tsSpecularTerm = 1.0 / 4 / info.cosThetaIn / info.cosThetaOut;

				// 3. Calculate factor
				Color3f result =  this->calSpecular(out, in, info);

				possibility = info.Ds * info.cosThetaH / 4 / info.cosThetaD * (1-pclearcoat);
				return result;
			}
		}
		else {
			//#TODO2 同时采样diffuse和specular
			CHECK(0);
		}

		return Color3f(0);
	}

	Float DisneyBrdf::calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		//#TODO2
		CHECK(0);
		return 0;
	}

	mcl::Color3f DisneyBrdf::albedo() const
	{
		return baseColor;
	}

	mcl::Color3f DisneyBrdf::calDiffuse(const Vector3f& out, const Vector3f& in, ScatterInfo& info) const
	{
		/* calculate diffuse term */
		const Float cosThetaIn_m1_5 = pow5(1 - info.cosThetaIn);
		const Float cosThetaOut_m1_5 = pow5(1 - info.cosThetaOut);
		const Float cosThetaD_2 = info.cosThetaD * info.cosThetaD;
		//calculate 'Lambertian' term
		const Float fd90 = 0.5 + 2 * cosThetaD_2 * roughness;
		const Float fd = (1 + (fd90 - 1)*cosThetaIn_m1_5) * (1 + (fd90 - 1)*cosThetaOut_m1_5);
		//calculate subsurface term
		const Float Fss90 = cosThetaD_2 * roughness;
		const Float Fss = (1 + (Fss90 - 1)*cosThetaIn_m1_5)*(1 + (Fss90 - 1)*cosThetaOut_m1_5);
		const Float fss = 1.25 * (Fss * (1 / (info.cosThetaOut + info.cosThetaIn) - 0.5) + 0.5);
		//calculate sheen term
		const Color3f fsh = interpolate(Color3f(1, 1, 1), colorTint, sheenTint) * sheen * info.cosThetaD_m1_5;

		return (1 - metallic)*(baseColor * INVPI * interpolate(fd, fss, subsurface) + fsh);
	}
																											
	mcl::Color3f DisneyBrdf::calSpecular(const Vector3f& out, const Vector3f& in, ScatterInfo& info) const
	{
		/* calculate specular term */
		//calculate Fresnel term
		const Color3f fresnel_s = (Color3f(1, 1, 1) - cR0) * info.cosThetaD_m1_5 + cR0;
		//calculate G term
		auto Gs1 = [&](const Float& cosPhi, const Float& sinPhi, const Float& tanTheta) {
			Float lambda = (sqrt(1 + (alphaX*alphaX*cosPhi*cosPhi + alphaY * alphaY*sinPhi*sinPhi)*tanTheta*tanTheta) - 1) / 2;
			return 1 / (1 + lambda);
		};
		const Float Gs = Gs1(info.cosPhiOut, info.sinPhiOut, info.tanThetaOut) * Gs1(info.cosPhiIn, info.sinPhiIn, info.tanThetaIn);
		//calculate D term
		if (alphaX < FloatZero) {
			info.Ds = 1;
			return (info.sinThetaH < FloatZero)? fresnel_s * info.tsSpecularTerm : Color3f(0);
		}
		else {
			info.Ds = 1 / (PI * alphaX * alphaY * pow2(info.sinThetaH * info.sinThetaH * info.PHI + info.cosThetaH * info.cosThetaH));
			return fresnel_s * Gs * info.Ds * info.tsSpecularTerm;
		}

	}

	mcl::Color3f DisneyBrdf::calClearcoat(const Vector3f& out, const Vector3f& in, ScatterInfo& info) const
	{
		/* calculate clearcoat term */
		//calculate Fresnel term
		const Float fresnel_c = 0.04 + 0.96 * info.cosThetaD_m1_5;
		//calculate G term
		auto G1c = [this](const Float& tanTheta) {
			Float lambda = (sqrt(1 + this->alpha_c * tanTheta * tanTheta) - 1) / 2;
			return 1 / (1 + lambda);
		};
		const Float Gc = G1c(info.tanThetaIn)*G1c(info.tanThetaOut);
		//calculate D term
		info.Dc = (alpha_c - 1) / (PI * std::log(alpha_c) * (alpha_c * info.cosThetaH*info.cosThetaH + info.sinThetaH * info.sinThetaH));

		return Color3f(clearcoat / 4 * fresnel_c*Gc*info.Dc * info.tsSpecularTerm);
	}

}