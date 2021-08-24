#include "Light.h"

namespace mcl {
	ConstantLight::ConstantLight(Color3f Ambient, Color3f Diffuse, Color3f Specular,Point3f pos, PositionType posType)
		:ambient_(Ambient), diffuse_(Diffuse), specular_(Specular), pos_(pos), Light(posType)
	{
	}

	mcl::Point3f ConstantLight::getPosition(Transform invViewMatrix)
	{
		if (posType_ == Light::L_ABSOLUTE) {
			return pos_;
		}
		else {
			return invViewMatrix * pos_;
		}
	}

}

