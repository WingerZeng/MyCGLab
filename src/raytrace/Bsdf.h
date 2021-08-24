#pragma once
#include "mcl.h"
#include "types.h"
#include "Sampler.h"
namespace mcl {
	enum SurfaceType
	{
		NULL_Surface = 0,
		Specular = 1,
		Diffuse = 2,
		ALL_Surface = 3,
	};

	enum ScatterType
	{
		NULL_Scatter = 0,
		Reflect = 1,
		Refract = 2,
		ALL_Scatter = 3,
	};

	class Bsdf
	{
	public:
		Bsdf(SurfaceType sft, ScatterType sct)
			:sft(sft), sct(sct) {}
		Bsdf()
			:sft(NULL_Surface), sct(NULL_Scatter) {}
		SurfaceType surfaceType() const { return sft; }
		ScatterType scatterType() const { return sct; }
		virtual ~Bsdf() {};
		inline bool hasType(SurfaceType isft, ScatterType isct) const;
		inline ScatterType activeScatterType(ScatterType isct) const;
		inline SurfaceType activeSurfaceType(SurfaceType isft) const;

		//调用下述接口时，方向向量统一从表面点指向表面外, 子类默认ScatterType中的信息正确，不会检查两个光线位置是否满足反射或透射关系
		// in与out向量可位于表面的任意侧
		virtual Color3f calF(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const = 0;
		virtual Color3f calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const = 0;
		virtual Float calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const = 0;
		virtual Color3f albedo() const { return Color3f(0); }
		virtual SamplerRequestInfo getSamplerRequest() const { return SamplerRequestInfo(); }
	protected:
		SurfaceType sft;
		ScatterType sct;
	};

	inline bool Bsdf::hasType(SurfaceType isft, ScatterType isct) const {
		return ((sft & isft) && (sct & isct));
	}

	inline ScatterType Bsdf::activeScatterType(ScatterType isct) const {
		return ScatterType(isct & sct);
	}

	inline SurfaceType Bsdf::activeSurfaceType(SurfaceType isft) const {
		return SurfaceType(isft & sft);
	}

	class BsdfGroup : public Bsdf
	{
	public:
		/**
		 * @brief nvec, uvec为法向量与u方向向量, 假定都已经单位化
		 */
		BsdfGroup(const Vector3f& nvec, const Vector3f& uvec, const Vector3f& geo_nvec);
		
		void addBsdf(std::unique_ptr<Bsdf> bsdf);

		/*
		*@brief 该函数将向量从世界坐标系转换到表面坐标系，该坐标系下表面法向为z方向，uv分别为xy方向
		*/
		Vector3f worldToSurface(const Vector3f& wvec) const;
		Vector3f surfaceToWorld(const Vector3f& svec) const;

		virtual Color3f calF(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;
		virtual Color3f calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;
		virtual Float calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft = ALL_Surface, ScatterType sct = ALL_Scatter) const override;
		virtual SamplerRequestInfo getSamplerRequest() const;
		virtual Color3f albedo() const override;

	private:
		Transform toSurface, toWorld;
		const Vector3f geonvec;
		std::vector<std::unique_ptr<Bsdf>> bsdfs;
	};

	inline bool calRefract(const Vector3f& out, const Vector3f& in, const Float& etaOut, const Float& eatIn) {
		
	}
}

