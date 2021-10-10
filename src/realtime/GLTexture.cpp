#include "GLTexture.h"
#include "GLFunctions.h"
namespace mcl{
	
	GLAbstractTexture::GLAbstractTexture()
	{
		GLFUNC->glGenTextures(1, &tex);
		if(DestroyedActiveId.empty())
			activedTexId = ActivedTextureCnt++;
		else {
			activedTexId = DestroyedActiveId.back();
			DestroyedActiveId.pop_back();
		}
	}

	GLAbstractTexture::~GLAbstractTexture()
	{
		GLFUNC->glDeleteTextures(1, &tex);
		DestroyedActiveId.push_back(activedTexId);
	}


	void GLAbstractTexture::resize(int w, int h)
	{
		this->w = w;
		this->h = h;
	}

	GLuint GLAbstractTexture::ActivedTextureCnt = 0;

	std::vector<GLuint> GLAbstractTexture::DestroyedActiveId;

	GLTexture2D::GLTexture2D(GLuint internalType, GLuint baseType, GLuint unitType)
		:internalType(internalType), baseType(baseType), unitType(unitType)
	{
	}

	void GLTexture2D::bindToUniform(std::string name, QOpenGLShaderProgram* shader)
	{
		GLFUNC->glActiveTexture(GL_TEXTURE0 + activedTexId);
		GLFUNC->glBindTexture(GL_TEXTURE_2D, tex);
		shader->setUniformValue(name.c_str(), activedTexId);
	}

	void GLTexture2D::bind()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D, tex);
	}

	void GLTexture2D::release()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);
	}

	void GLTexture2D::resize(int w, int h)
	{
		GLAbstractTexture::resize(w, h);
		GLFUNC->glBindTexture(GL_TEXTURE_2D, tex);
		GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, internalType, w, h, 0, baseType, unitType, NULL);
	}

	void GLTexture2D::bindToFbo(GLuint fbo, GLuint attachment)
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex, 0);
	}

	void GLTexture2D::setFilter(GLuint minFilter, GLuint maxFilter)
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D, tex);
		GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxFilter);
		GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);
	}

	void GLTexture2D::setWrap(GLuint s, GLuint t)
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D, tex);
		GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s);
		GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
		GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);
	}

	GLTextureMultiSample::GLTextureMultiSample(GLuint internalType, int nsample)
		:internalType(internalType), nsample(nsample)
	{
	}

	void GLTextureMultiSample::bindToUniform(std::string name, QOpenGLShaderProgram* shader)
	{
		GLFUNC->glActiveTexture(GL_TEXTURE0 + activedTexId);
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
		shader->setUniformValue(name.c_str(), activedTexId);
	}

	void GLTextureMultiSample::bind()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	}

	void GLTextureMultiSample::release()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}

	void GLTextureMultiSample::resize(int w, int h)
	{
		GLAbstractTexture::resize(w, h);
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
		GLFUNC->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nsample, internalType, w, h, GL_TRUE);
	}

	void GLTextureMultiSample::bindToFbo(GLuint fbo, GLuint attachment)
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
	}

	GLTextureCubeMap::GLTextureCubeMap(GLuint internalType, GLuint baseType, GLuint unitType)
		:internalType(internalType), baseType(baseType), unitType(unitType)
	{

	}

	void GLTextureCubeMap::bindToUniform(std::string name, QOpenGLShaderProgram* shader)
	{
		GLFUNC->glActiveTexture(GL_TEXTURE0 + activedTexId);
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
		shader->setUniformValue(name.c_str(), activedTexId);
	}

	void GLTextureCubeMap::bind()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	}

	void GLTextureCubeMap::release()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void GLTextureCubeMap::resize(int w, int h)
	{
		GLAbstractTexture::resize(w, h);
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
		for (GLuint i = 0; i < 6; i++) {
			GLFUNC->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalType, w, h, 0, baseType, unitType, NULL);
		}
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void GLTextureCubeMap::bindToFbo(GLuint fbo, GLuint attachment)
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLFUNC->glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex, 0);
	}

	void GLTextureCubeMap::setFilter(GLuint minFilter, GLuint maxFilter)
	{
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, maxFilter);
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void GLTextureCubeMap::setWrap(GLuint s, GLuint t, GLuint r)
	{
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, s);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, t);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, r);
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

}