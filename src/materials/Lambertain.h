#pragma once
#include "mcl.h"
#include "types.h"
#include "Bsdf.h"
#include "Material.h"
#include "Texture.h"
class DataNode;
namespace mcl {
	class LambertainBsdf:public Bsdf
	{
	public:
		LambertainBsdf(const Color3f& factor, ScatterType sct = Reflect);

		virtual Color3f calF(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Color3f calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual Float calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;


		virtual SamplerRequestInfo getSamplerRequest() const override;


		virtual Color3f albedo() const override;

	private:
		Color3f factor;
	};

	class LambertainMaterial :public Material {
	public:
		LambertainMaterial(Color3f diffuse);
		LambertainMaterial(const std::shared_ptr<Texture<Color3f>>& diffuse);
		LambertainMaterial(DataNode* data);
		virtual std::unique_ptr<BsdfGroup> getBsdfs(HitRecord* rec, Sampler& sampler) const override;

		virtual SamplerRequestInfo getSamplerRequest() const override {
			return SamplerRequestInfo(0, 1);
		}

		virtual void initGL() override;

		virtual void prepareGL(QOpenGLShaderProgram* shader) override;

	private:
		std::shared_ptr<Texture<Color3f>> diffuse;

		//For real-time
		ParameterType kdtype;
		QVector4D kd;
		QString kd_map;
	};
}

