#include "GLFunctions.h"

namespace mcl {
	QOpenGLFunctions_4_3_Core* GLFunctions::getSingletonPtr()
	{
		static QOpenGLFunctions_4_3_Core* glf = new QOpenGLFunctions_4_3_Core;
		return glf;
	}
}