#include "RTScene.h"
#include "RTLight.h"
#include "RTPrimitive.h"
#include "RTCamera.h"
#include "Bsdf.h"
#include "Material.h"
#include "Record.h"
namespace mcl{
	
	RTScene::RTScene(const std::vector<std::shared_ptr<RTPrimitive>>& primitives, const std::vector<std::shared_ptr<RTLight>>& lights, const std::shared_ptr<RTCamera>& camera)
		:primitives(primitives),lights(lights),camera(camera)
	{

	}
	 
	bool RTScene::intersect(const Ray& ray, HitRecord* hit) const
	{  
		bool find = false;
		Ray tempray = ray;
		for (const auto& primitive : primitives) {
			if (primitive->intersect(tempray, hit)) {
				tempray.t1 = hit->t;
				find = true;
			}
		}
		return find;
	}

	bool RTScene::intersectShadow(const Ray& ray) const
	{
		for (const auto& primitive : primitives) {
			if (primitive->intersectShadow(ray)) {
				return true;
			}
		}
		return false;
	}

	mcl::SamplerRequestInfo RTScene::getSamplerRequest() const
	{
		SamplerRequestInfo ret;
		for (const auto& prim : primitives) {
			ret = ret + prim->getSamplerRequest();
		}
		for (const auto& light : lights) {
			ret = ret + light->getSamplerRequest();
		}
		ret = ret + camera->getSamplerRequest();
		return ret;
	}

}