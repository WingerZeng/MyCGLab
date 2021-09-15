#include "BlinnPhongMaterial.h"
#include "algorithms.h"
namespace mcl{
	
	BlinnPhongMaterial::BlinnPhongMaterial(const std::shared_ptr<Texture<Color3f>>& kd, const std::shared_ptr<Texture<Color3f>>& ks, const std::shared_ptr<Texture<Float>>& ns)
		:kd(kd),ks(ks),ns(ns)
	{
	}

	BlinnPhongMaterial::BlinnPhongMaterial(DataNode* data)
	{
		kd = getColorTexture("Kd", data);
		ks = getColorTexture("Ks", data);
		ns = getFloatTexture("Ns", data);
		kdtype = getColorTexture("Kd", data, kd_vec, kd_map);
		kstype = getColorTexture("Ks", data, ks_vec, ks_map);
		nstype = getFloatTexture("Ns", data, ns_float, ns_map);
	}

	std::unique_ptr<mcl::BsdfGroup> BlinnPhongMaterial::getBsdfs(HitRecord* rec, Sampler& sampler) const
	{
		assert(rec->n.normalized());
		bumpNormal(rec);
		std::unique_ptr<BsdfGroup> bsdfs(new BsdfGroup(Vector3f(rec->bumped_n), Normalize(rec->uvec), Vector3f(rec->n)));
		bsdfs->addBsdf(std::unique_ptr<Bsdf>(new BlinnPhongBsdf(kd->value(*rec), ks->value(*rec), ns->value(*rec))));
		return bsdfs;
	}

	mcl::SamplerRequestInfo BlinnPhongMaterial::getSamplerRequest() const
	{
		return SamplerRequestInfo(0, 1);
	}

	void BlinnPhongMaterial::initGL()
	{
		//#TODO1 初始化纹理
	}

	void BlinnPhongMaterial::prepareGL(QOpenGLShaderProgram* shader)
	{
		//#TODO1 装载纹理
		shader->setUniformValue("Le", QVector4D(QVector3D(getEmission()),1));
		shader->setUniformValue("MatType", (GLint)_type);
		if (kdtype & P_Color) {
			shader->setUniformValue("ourColor", kd_vec);
			shader->setUniformValue("Kd", kd_vec);
		}
		if (kstype & P_Color) {
			shader->setUniformValue("Ks", ks_vec);
		}
		if (nstype & P_Float) {
			shader->setUniformValue("Ns", ns_float);
		}
	}

	BlinnPhongBsdf::BlinnPhongBsdf(const Color3f& kd, const Color3f& ks, const Float& ns)
		:kd(kd),ks(ks),ns(ns),Bsdf(NULL_Surface, Reflect)
	{
		diffusePossibility = std::clamp( kd.length() / (kd.length() + ks.length()),0.0f,1.0f);
		Color3f add = kd + ks;
		Float maxdim = add[add.absMaxDim()];
		if (maxdim > 1) {	//为了防止违反能量条件，对kd与ks参数的总和进行约束
			this->kd = this->kd / maxdim;
			this->ks = this->ks / maxdim;
		}

		if (diffusePossibility >= 1) sft = Diffuse;
		else if (diffusePossibility <= FloatZero) sft = Specular;
		else sft = ALL_Surface;
	}

	mcl::Color3f BlinnPhongBsdf::calF(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		if (!hasType(sft, sct)) return Color3f(0);
		Vector3f a = out + in;
		Vector3f h = a / a.length();
		Float specular;
		switch (activeSurfaceType(sft))
		{
		case ALL_Surface:
			specular = std::pow(std::abs(h.z()), ns);
			return kd * INVPI + ks * specular * (ns + 1) * INVPI / 8 / std::abs(in.z()) / std::abs(out.z()); //为了保证能量守恒，此处除以Pi
		case Diffuse:
			return kd * INVPI; //为了保证能量守恒，此处除以Pi
		case Specular:
			specular = std::pow(std::abs(h.z()), ns);
			//return ks * specular * (ns + 1) * INVPI / 8 / std::abs(in.z()) / std::abs(out.z());
			return ks * specular * (ns + 1) * INVPI / 8 / std::abs(in.z());
		default:
			assert(0);
			return Color3f(0);
		}
	}

	mcl::Color3f BlinnPhongBsdf::calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType isft /*= ALL_Surface*/, ScatterType isct /*= ALL_Scatter*/) const
	{
		if (!hasType(isft, isct)) {
			possibility = 0;
			return Color3f(0);
		}

		Point2f sample = sampler.get2DSample();

		auto asft = activeSurfaceType(isft);
		if (asft == Diffuse) {
			// 重要性采样漫反射项
			in = cosSampleHemisphere(sample);
			in.z() *= out.z() > 0 ? 1 : -1;
			possibility = calAbsCosTheta(in)*INVPI;
			//std::cout << "Diffuse" << kd * INVPI / possibility << std::endl;
			return kd * INVPI;
		}
		else if (asft == Specular) {
			// 重要性采样高光项
			Float hphi = PI_2 * sample.y();
			Float htheta = std::acos(std::pow(sample.x(), 1 / (ns + 1)));
			Vector3f h = SphereToXYZ(hphi, htheta);
			in = reflectByNormal(out, h);
			in.z() *= out.z() > 0 ? 1 : -1;
			if (in.z() * out.z() <= 0) {
				return Color3f(0);
			}
			Float costhetan = std::pow(std::abs(h.z()), ns);
			possibility = (ns) / (8 * out.absDot(h)) * costhetan*INVPI;
			//return ks * costhetan * (ns + 1) * INVPI / 8 / std::abs(in.z()) / std::abs(out.z());
			return ks * costhetan * (ns + 1) * INVPI / 8 / std::abs(in.z());
		}
		else {
			Float diffuseSample = sample.x();
			sample.x() *= 10;
			sample.x() -= floor(sample.x());
			if (diffuseSample >= diffusePossibility) { // choose specular lobe
				// 重要性采样高光项
				Float hphi = PI_2 * sample.y();
				Float htheta = std::acos(std::pow(sample.x(), 1 / (ns)));
				Vector3f h = SphereToXYZ(hphi, htheta);
				in = reflectByNormal(out, h);
				in.z() *= out.z() > 0 ? 1 : -1;
				if (in.z() * out.z() <= 0) {
					return Color3f(0);
				}
				Float costhetan = std::pow(std::abs(h.z()), ns);
				//possibility = (ns + 1) / (8 * out.absDot(h)) * costhetan*INVPI*(1 - diffusePossibility);
				possibility = (ns) / (8 * out.absDot(h)) * costhetan*INVPI * (1 - diffusePossibility);
				//return  ks * costhetan * (ns + 1) * INVPI / 8 / std::abs(in.z()) / std::abs(out.z());
				return ks * costhetan * (ns + 1) * INVPI / 8 / std::abs(in.z());
			}
			else { // choose diffuse lobe
				// 重要性采样漫反射项
				in = cosSampleHemisphere(sample);
				in.z() *= out.z() > 0 ? 1 : -1;
				possibility = calAbsCosTheta(in)*INVPI*diffusePossibility;
				return kd * INVPI;
			}
		}
	}

	Float BlinnPhongBsdf::calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType isft /*= ALL_Surface*/, ScatterType isct /*= ALL_Scatter*/) const
	{
		if (!hasType(isft, isct)) {
			return 0;
		}
		// #TODO2
		CHECK(0);
		return calAbsCosTheta(in)*INVPI;
	}

	mcl::Color3f BlinnPhongBsdf::albedo() const
	{
		return kd+ks;
	}

	mcl::SamplerRequestInfo BlinnPhongBsdf::getSamplerRequest() const
	{
		return SamplerRequestInfo(0, 1);
	}

}