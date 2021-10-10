#include "GLFunctions.h"

namespace mcl {
	GLFunctions* GLFunctions::getSingletonPtr()
	{
		static GLFunctions* glf = new GLFunctions;
		return glf;
	}
}