#include "RTPrimitive.h"
#include "Geometry.h"
#include "Material.h"
#include "Record.h"
#include "Bsdf.h"

namespace mcl{
	
	mcl::SamplerRequestInfo RTPrimitive::getSamplerRequest() const
	{
		return SamplerRequestInfo();
	}

	RTGeometryPrimitive::RTGeometryPrimitive(std::shared_ptr<Geometry> geo, std::shared_ptr<Material> mat, std::shared_ptr<RTSurfaceLight> light)
		:geo(geo),mat(mat),light(light)
	{

	}

	mcl::Material* RTGeometryPrimitive::getMaterial() const
	{
		return mat.get();
	}

	std::unique_ptr<mcl::BsdfGroup> RTGeometryPrimitive::getBsdf(HitRecord* rec, Sampler& sampler) const
	{
		if (mat) {
			return mat->getBsdfs(rec, sampler);
		}
		else {
			return nullptr;
		}
	}

	bool RTGeometryPrimitive::intersect(const Ray& ray, HitRecord* rec)
	{
		if (geo->intersect(ray, rec)) {
			rec->prim = this;
			return true;
		}
		return false;
	}

	bool RTGeometryPrimitive::intersectShadow(const Ray& ray)
	{
		return geo->intersectShadow(ray);
	}

}