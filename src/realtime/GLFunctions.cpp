#include "GLFunctions.h"

namespace mcl {
	GLFunctions* GLFunctions::getSingletonPtr()
	{
		static GLFunctions* glf = new GLFunctions;
		return glf;
	}

	void GLFunctions::activeUniformTexture(GLuint target, GLuint texid, std::string name, QOpenGLShaderProgram* shader)
	{
		//#TODO1 texture整理为texture类,在类中分配active texture,不要在每次渲染循环时分配
		GLFUNC->glActiveTexture(GL_TEXTURE0 + activedTexture);
		GLFUNC->glBindTexture(target, texid);
		shader->setUniformValue(name.c_str(), activedTexture);
		activedTexture++;
	}

	void GLFunctions::resetActiveTexture()
	{
		activedTexture = 0;
	}

}