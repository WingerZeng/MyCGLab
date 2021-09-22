#pragma once
#include "mcl.h"
#include "types.h"
#include <QOpenGLFunctions_4_3_Core>
#define GLFUNC mcl::GLFunctions::getSingletonPtr()
namespace mcl {
	class GLFunctions
	{
	public:
		static QOpenGLFunctions_4_3_Core* getSingletonPtr();
	};
}
