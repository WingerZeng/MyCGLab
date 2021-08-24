#include "Mirror.h"
#include "Record.h"
#include "algorithms.h"
namespace mcl{
	
	MirrorBsdf::MirrorBsdf(Color3f factor)
		:Bsdf(Specular,Reflect),factor(factor)
	{
	}

	mcl::Color3f MirrorBsdf::calF(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (hasType(sft, sct)&&isMirror(out,in))
			return factor/calAbsCosTheta(in); //由于bsdf项之后乘了cos，此处要除cos抵消
		else
			return Color3f(0);
	}

	mcl::Color3f MirrorBsdf::calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct)) {
			possibility = 0;
			return Color3f(0);
		}
		in = calReflect(out);
		possibility = 1;
		return factor/calAbsCosTheta(in);
	}

	Float MirrorBsdf::calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (hasType(sft,sct)) 
			return 1;
		else
			return 0;
	}

	MirrorMaterial::MirrorMaterial(const Color3f& specular)
		:specular(std::make_shared<ConstantTexture<Color3f>>(specular))
	{
	}

	MirrorMaterial::MirrorMaterial(const std::shared_ptr<Texture<Color3f>>& specular)
		:specular(specular)
	{
	}

	MirrorMaterial::MirrorMaterial(DataNode* data)
	{
		specular = getColorTexture("Kr",data);
	}

	std::unique_ptr<mcl::BsdfGroup> MirrorMaterial::getBsdfs(HitRecord* rec, Sampler& sampler) const
	{
		assert(rec->n.normalized());
		bumpNormal(rec);
		std::unique_ptr<BsdfGroup> bsdfs(new BsdfGroup(Vector3f(rec->bumped_n), Normalize(rec->uvec), Vector3f(rec->n)));
		bsdfs->addBsdf(std::unique_ptr<Bsdf>(new MirrorBsdf(specular->value(*rec))));
		return bsdfs;
	}

}