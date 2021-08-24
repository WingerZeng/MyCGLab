#pragma once
#include "mcl.h"
#include "types.h"
#include "Bsdf.h"
#include "Material.h"
#include "Texture.h"
class DataNode;
namespace mcl {
	class DielectricBsdf:public Bsdf
	{
	public:
		DielectricBsdf(const Color3f& reflFactor, const Color3f& refrFactor, Float eta1, Float eta2);

		virtual Color3f calF(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Color3f calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Float calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;

		virtual SamplerRequestInfo getSamplerRequest() const override { return SamplerRequestInfo(1,0); }

		Float calReflectFresnel(const Float& outCosTheta, const Float& inCosTheta, const Float& etaout, const Float& etain) const;
	private:
		Float eta1, eta2; //eta1所在介质与法向同侧，即 z>0 侧
		const Float ReflectPossibility = 0.5;
		Color3f reflFactor, refrFactor;
	};

	class DielectricMaterial :public Material
	{
	public:
		DielectricMaterial(Float eta1, Float eta2, const Color3f& reflFactor = Color3f(1), const Color3f& refrFactor = Color3f(1));
		DielectricMaterial(Float eta1, Float eta2, const std::shared_ptr<Texture<Color3f>>& reflFactor, const std::shared_ptr<Texture<Color3f>>& refrFactor);
		DielectricMaterial(DataNode* data);

		virtual std::unique_ptr<BsdfGroup> getBsdfs(HitRecord* rec, Sampler& sampler) const override;

		virtual SamplerRequestInfo getSamplerRequest() const override { return SamplerRequestInfo(1, 0); }

	private:
		std::shared_ptr<Texture<Color3f>> reflFactor;
		std::shared_ptr<Texture<Color3f>> refrFactor;
		Float eta1, eta2;
	};
}

