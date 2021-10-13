#pragma once
#include "mcl.h"
#include "types.h"
#include "Material.h"
#include "Bsdf.h"
#include "Texture.h"
class DataNode;
namespace mcl {
	class BlinnPhongMaterial:public Material
	{
	public:
		BlinnPhongMaterial(const std::shared_ptr<Texture<Color3f>>& kd, const std::shared_ptr<Texture<Color3f>>& ks, const std::shared_ptr<Texture<Float>>& ns);
		BlinnPhongMaterial(DataNode* data);

		virtual std::unique_ptr<BsdfGroup> getBsdfs(HitRecord* rec, Sampler& sampler) const override;

		virtual SamplerRequestInfo getSamplerRequest() const override;


		virtual void initGL() override;

		virtual void prepareGL(QOpenGLShaderProgram* shader) override;

	private:
		std::shared_ptr<Texture<Color3f>> kd;
		std::shared_ptr<Texture<Color3f>> ks;
		std::shared_ptr<Texture<Float>> ns;
		DataNode* data;

		//for real-time
		std::shared_ptr<GLTexture2D> glkd;
		std::shared_ptr<GLTexture2D> glks;
		std::shared_ptr<GLTexture2D> glns;
	};


	class BlinnPhongBsdf : public Bsdf
	{
	public:
		BlinnPhongBsdf(const Color3f& kd, const Color3f& ks, const Float& ns);



		virtual Color3f calF(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Color3f calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Float calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Color3f albedo() const override;


		virtual SamplerRequestInfo getSamplerRequest() const override;

	private:
		Color3f kd, ks;
		Float diffusePossibility;
		Float ns;
	};
}

