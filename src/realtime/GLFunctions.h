#pragma once
#include "mcl.h"
#include "types.h"
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLShaderProgram>
#define GLFUNC mcl::GLFunctions::getSingletonPtr()
namespace mcl {
	class GLFunctions: public QOpenGLFunctions_4_3_Core
	{
	public:
		static GLFunctions* getSingletonPtr();
		void activeUniformTexture(GLuint target, GLuint texid, std::string name, QOpenGLShaderProgram* shader);
		void resetActiveTexture();
	protected:
		GLFunctions() {};
	private:
		int activedTexture = 0;
	};
}
