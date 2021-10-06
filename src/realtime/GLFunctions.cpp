#include "GLFunctions.h"

namespace mcl {
	GLFunctions* GLFunctions::getSingletonPtr()
	{
		static GLFunctions* glf = new GLFunctions;
		return glf;
	}

	void GLFunctions::activeUniformTexture(GLuint target, GLuint texid, std::string name, QOpenGLShaderProgram* shader)
	{
		//#TODO1 texture����Ϊtexture��,�����з���active texture,��Ҫ��ÿ����Ⱦѭ��ʱ����
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