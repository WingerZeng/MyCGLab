#pragma once
#include "mcl.h"
#include "types.h"
#include "Bsdf.h"
#include "Material.h"
#include "Texture.h"
namespace mcl {
	class MirrorBsdf:public Bsdf
	{
	public:
		MirrorBsdf(Color3f factor = Color3f(1));

		virtual Color3f calF(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Color3f calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Float calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;

	protected:
		Color3f factor;
	};

	class MirrorMaterial :public Material
	{
	public:
		MirrorMaterial(const Color3f& specular);
		MirrorMaterial(DataNode* data);
		MirrorMaterial(const std::shared_ptr<Texture<Color3f>>& specular);
		virtual std::unique_ptr<BsdfGroup> getBsdfs(HitRecord* rec, Sampler& sampler) const override;
	private:
		std::shared_ptr<Texture<Color3f>> specular;

		//#TODO1 real time Œ∆¿Ì
	};
}

