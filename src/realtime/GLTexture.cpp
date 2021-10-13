#include "GLTexture.h"
#include "GLFunctions.h"
#include "stb_image.h"
//#TEST
#include "MainWindow.h"
#include "Scene.h"
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

	std::shared_ptr<mcl::GLTexture2D> GLTexture2D::createColorGLTexture2D(Color3f scale, const QString& texture, bool sRGB)
	{
		GLuint ptype;
		GLuint basetype;
		void* texdata;
		int width, height;
		int nrChannels;
		std::shared_ptr<mcl::GLTexture2D> tex; 
		stbi_set_flip_vertically_on_load(true);
		if (stbi_is_hdr(texture.toStdString().c_str())) {
			float *data = stbi_loadf(texture.toStdString().c_str(), &width, &height, &nrChannels, 0);
			texdata = data;
			if (nrChannels == 3) {
				ptype = GL_RGB32F;
				basetype = GL_RGB;
			}
			else {
				ptype = GL_RGBA32F;
				basetype = GL_RGBA;
			}
			float * pnt = data;
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					for (int channel = 0; channel < 3; channel++) {
						*pnt = *pnt * scale[channel];
						pnt++;
					}
					if (nrChannels == 4) {
						pnt++;
					}
				}
			}
			tex = std::make_shared<mcl::GLTexture2D>(ptype, basetype, GL_FLOAT);
			tex->bind();
			GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, ptype, width, height, 0, basetype, GL_FLOAT, texdata);
			stbi_image_free(data);
		}
		else {
			unsigned char *data = stbi_load(texture.toStdString().c_str(), &width, &height, &nrChannels, 0);
			texdata = data;
			if (nrChannels == 3) {
				ptype = sRGB?GL_SRGB:GL_RGB;
				basetype = GL_RGB;
			}
			else {
				ptype = sRGB ? GL_SRGB_ALPHA : GL_RGBA;
				basetype = GL_RGBA;
			}
			unsigned char * pnt = data;
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					for (int channel = 0; channel < 3; channel++) {
						*pnt = unsigned char(*pnt * scale[channel]);
						pnt++;
					}
					if (nrChannels == 4) {
						pnt++;
					}
				}
			}
			tex = std::make_shared<mcl::GLTexture2D>(ptype, basetype, GL_UNSIGNED_BYTE);
			tex->bind();
			GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, ptype, width, height, 0, basetype, GL_UNSIGNED_BYTE, texdata);
			stbi_image_free(data);
		}
		GLFUNC->glGenerateMipmap(GL_TEXTURE_2D);
		tex->setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		tex->setWrap(GL_REPEAT, GL_REPEAT);
		return tex;
	}

	std::shared_ptr<mcl::GLTexture2D> GLTexture2D::createColorGLTexture2D(Color3f color)
	{
		GLuint ptype = GL_RGB32F;
		GLuint basetype = GL_RGB;
		std::shared_ptr<mcl::GLTexture2D> tex = std::make_shared<mcl::GLTexture2D>(ptype, basetype, GL_FLOAT);
		tex->bind();
		std::vector<Float> texdata{color[0], color[1], color[2]};
		GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, ptype, 1, 1, 0, basetype, GL_FLOAT, &texdata[0]);
		tex->setFilter(GL_NEAREST, GL_NEAREST);
		tex->setWrap(GL_REPEAT, GL_REPEAT);
		return tex;
	}

	std::shared_ptr<mcl::GLTexture2D> GLTexture2D::createFloatGLTexture2D(Float scale, const QString& texture)
	{
		GLuint ptype;
		GLuint basetype;
		void* texdata;
		int width, height;
		int nrChannels;
		std::shared_ptr<mcl::GLTexture2D> tex;
		stbi_set_flip_vertically_on_load(true);
		if (stbi_is_hdr(texture.toStdString().c_str())) {
			float *data = stbi_loadf(texture.toStdString().c_str(), &width, &height, &nrChannels, 1);
			texdata = data;
			ptype = GL_R32F;
			basetype = GL_RED;
			float * pnt = data;
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					*pnt = *pnt * scale;
					pnt++;
				}
			}
			tex = std::make_shared<mcl::GLTexture2D>(ptype, basetype, GL_FLOAT);
			tex->bind();
			GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, ptype, width, height, 0, basetype, GL_FLOAT, texdata);
			stbi_image_free(data);
		}
		else {
			unsigned char *data = stbi_load(texture.toStdString().c_str(), &width, &height, &nrChannels, 1);
			texdata = data;
			ptype = GL_RED;
			basetype = GL_RED;
			unsigned char * pnt = data;
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					*pnt = unsigned char(*pnt * scale);
					pnt++;
				}
			}
			tex = std::make_shared<mcl::GLTexture2D>(ptype, basetype, GL_UNSIGNED_BYTE);
			tex->bind();
			GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, ptype, width, height, 0, basetype, GL_UNSIGNED_BYTE, texdata);
			stbi_image_free(data);
		}
		GLFUNC->glGenerateMipmap(GL_TEXTURE_2D);
		tex->setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		tex->setWrap(GL_REPEAT, GL_REPEAT);
		return tex;
	}

	std::shared_ptr<mcl::GLTexture2D> GLTexture2D::createFloatGLTexture2D(Float color)
	{
		GLuint ptype = GL_R32F;
		GLuint basetype = GL_RED;
		std::shared_ptr<mcl::GLTexture2D> tex = std::make_shared<mcl::GLTexture2D>(ptype, basetype, GL_FLOAT);
		tex->bind();
		GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, ptype, 1, 1, 0, basetype, GL_FLOAT, &color);
		tex->setFilter(GL_NEAREST, GL_NEAREST);
		tex->setWrap(GL_REPEAT, GL_REPEAT);
		return tex;
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