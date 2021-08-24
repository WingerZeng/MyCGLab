#pragma once
#include "mcl.h"
#include "types.h"
#include "Sampler.h"
namespace mcl {
	class RTScene
	{
	public:
		RTScene(const std::vector<std::shared_ptr<RTPrimitive>>& primitives,
			const std::vector<std::shared_ptr<RTLight>>& lights,
			const std::shared_ptr<mcl::RTCamera>& camera);
		
		bool intersect(const Ray& ray, HitRecord* hit) const;
		bool intersectShadow(const Ray& ray) const; //用于快速测试遮挡

		const std::vector<std::shared_ptr<mcl::RTLight>>& getLights() const { return lights; }
		const std::shared_ptr<mcl::RTCamera>& getCamera() const { return camera; }
		SamplerRequestInfo getSamplerRequest() const;
	private:
		std::vector<std::shared_ptr<RTPrimitive>> primitives;
		std::vector<std::shared_ptr<RTLight>> lights;
		std::shared_ptr<RTCamera> camera;
	};
}

