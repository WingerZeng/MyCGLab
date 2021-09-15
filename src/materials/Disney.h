#pragma once
#include "mcl.h"
#include "types.h"
#include "Material.h"
#include "Bsdf.h"
#include "Texture.h"
namespace mcl {
	struct DisneyMaterialInfo
	{
		std::shared_ptr<Texture<Color3f>> baseColor;
		std::shared_ptr<Texture<Float>> metallic;
		std::shared_ptr<Texture<Float>> subsurface;
		std::shared_ptr<Texture<Float>> specular;
		std::shared_ptr<Texture<Float>> specularTint;
		std::shared_ptr<Texture<Float>> roughness;
		std::shared_ptr<Texture<Float>> anisotropic;
		std::shared_ptr<Texture<Float>> sheen;
		std::shared_ptr<Texture<Float>> sheenTint;
		std::shared_ptr<Texture<Float>> clearcoat;
		std::shared_ptr<Texture<Float>> clearcoatGloss;
	}; 

	class DisneyMaterial: public Material
	{
	public:
		DisneyMaterial(const DisneyMaterialInfo& info);
		DisneyMaterial(DataNode* data);
		virtual std::unique_ptr<BsdfGroup> getBsdfs(HitRecord* rec, Sampler& sampler) const override;
		virtual SamplerRequestInfo getSamplerRequest() const { return SamplerRequestInfo(); }

	private:
		DisneyMaterialInfo info;
	};

	class DisneyBrdf: public Bsdf
	{
	public:
		DisneyBrdf(const Color3f& baseColor,
			const Float& metallic,
			const Float& subsurface,
			const Float& specular,
			const Float& specularTint,
			const Float& roughness,
			const Float& anisotropic,
			const Float& sheen,
			const Float& sheenTint,
			const Float& clearcoat,
			const Float& clearcoatGloss);

		virtual Color3f calF(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;

		virtual Color3f calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;;

		virtual Float calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;;

		virtual SamplerRequestInfo getSamplerRequest() const override { return SamplerRequestInfo(1,1); }

		virtual Color3f albedo() const override;

	private:
		struct ScatterInfo
		{
			//inputs
			Float cosThetaIn;
			Float cosThetaOut;

			Vector3f h;
			Float cosThetaD;
			Float cosThetaD_m1_5;

			Float cosPhiOut;
			Float cosPhiIn;
			Float sinPhiIn;
			Float sinPhiOut;
			Float tanThetaIn;
			Float tanThetaOut;
			Float cosThetaH;
			Float sinThetaH;
			Float tsSpecularTerm;

			Float PHI;

			//medium result of specular
			Float Ds;
			Float Dc;
		};

		Color3f calDiffuse(const Vector3f& out, const Vector3f& in, ScatterInfo& info) const;
		Color3f calSpecular(const Vector3f& out, const Vector3f& in, ScatterInfo& info) const; // calculate specular term without clearcoat
		Color3f calClearcoat(const Vector3f& out, const Vector3f& in, ScatterInfo& info) const;

		Color3f lum(const Color3f& color) const {
			Float l = (0.2126 * color[0] + 0.7152 * color[1] + 0.0722 * color[2]);
			return l > 0 ? color / l : Color3f(0);
		};


		const Color3f baseColor;
		const Float metallic;
		const Float subsurface;
		const Float specular;
		const Float specularTint;
		const Float roughness;
		const Float anisotropic;
		const Float sheen;
		const Float sheenTint;
		const Float clearcoat;
		const Float clearcoatGloss;

		const Float a;
		const Float alphaX;
		const Float alphaY;
		const Color3f colorTint;
		const Float pclearcoat;
		const Color3f cR0;
		const Float alpha_c;
	};

}

