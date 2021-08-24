#pragma once
#include "mcl.h"
#include "types.h"
namespace mcl {

	mcl::Point3f uniformSampleSphere(const Point2f& sample)
	{
		//Uniform sample sphere
		Float z = std::clamp(2 * sample.x() - 1, -1 + FloatZero, 1 - FloatZero);
		Float radius = std::sqrt(std::clamp(1 - z * z, 0.0f, 1.1f));
		Float phi = PI_2 * sample.y();
		return Point3f(radius * std::cos(phi), radius * std::sin(phi), z);
	}

}